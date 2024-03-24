/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOUpdateChecker.h"

#include <algorithm>
#include <iostream>
#include <thread>

#include <curl/curl.h>
#include <wx/regex.h>
#include <wx/string.h>
#include <yaml-cpp/yaml.h>

#include "threading/GOThread.h"
#include "yaml/go-wx-yaml.h"

#include "go_defs.h"
#include "settings.h"

wxDEFINE_EVENT(UPDATE_CHECKING_COMPLETION, GOUpdateChecker::CompletionEvent);

class GOVersion {
public:
  explicit GOVersion(const wxString &str) : m_components(), m_valid(false) {
    wxRegEx versionRegex(R"(v?(\d+).(\d+).(\d+)-(\d+).*)", wxRE_ADVANCED);
    if (versionRegex.Matches(str)) {
      assert(versionRegex.GetMatchCount() == m_components.size() + 1);
      for (size_t i = 0; i < m_components.size(); i++) {
        m_components[i] = std::atoi(versionRegex.GetMatch(str, i + 1).c_str());
      }
      m_valid = true;
    }
  }

  bool operator<(const GOVersion &other) const {
    return m_components < other.m_components;
  }

  bool IsValid() const { return m_valid; }

private:
  std::array<int, 4> m_components;
  bool m_valid;
};

class UpdateCheckerException : public std::runtime_error {
public:
  explicit UpdateCheckerException(const std::string &message)
    : std::runtime_error(message) {}
};

// a small helper class that destroys curl session in destructor
class CurlDestroyer {
public:
  explicit CurlDestroyer(CURL *curl) : m_curl(curl) {}
  ~CurlDestroyer() {
    if (m_curl) {
      curl_easy_cleanup(m_curl);
    }
  }

private:
  CURL *m_curl;
};

// a small helper class that destroys curl multi handle in destructor
class CurlMultiDestroyer {
public:
  explicit CurlMultiDestroyer(CURLM *curl) : m_curlMulti(curl) {}
  ~CurlMultiDestroyer() {
    if (m_curlMulti) {
      curl_multi_cleanup(m_curlMulti);
    }
  }

private:
  CURL *m_curlMulti;
};

static size_t handle_received_bytes(
  void *contents, size_t size, size_t nmemb, std::vector<char> *dst) {
  size_t bytesReceived = size * nmemb;
  char *contentsAsChars = static_cast<char *>(contents);
  dst->reserve(dst->size() + bytesReceived);
  dst->insert(dst->end(), contentsAsChars, contentsAsChars + bytesReceived);
  return bytesReceived;
}

struct GithubRelease {
  wxString name;
  wxString body;
  bool draft;
  bool prerelease;

  static GithubRelease fromYaml(const YAML::Node &node) {
    GithubRelease release;
    release.name = node["name"].as<wxString>();
    release.body = node["body"].as<wxString>();
    release.draft = node["draft"].as<bool>();
    release.prerelease = node["prerelease"].as<bool>();
    return release;
  }
};

class CheckForUpdatesThread : public GOThread {
public:
  explicit CheckForUpdatesThread(
    GOUpdateChecker::CheckReason checkReason,
    wxEvtHandler *completionEventHandler)
    : m_CompletionEventHandler(completionEventHandler),
      m_CheckReason(checkReason) {}

protected:
  void Entry() override {
    GOUpdateChecker::Result result
      = DoUpdateChecking().withCheckReason(m_CheckReason);
    m_CompletionEventHandler->QueueEvent(
      new GOUpdateChecker::CompletionEvent(UPDATE_CHECKING_COMPLETION, result));
  }

private:
  wxEvtHandler *m_CompletionEventHandler;
  GOUpdateChecker::CheckReason m_CheckReason;

  std::vector<char> FetchLatestReleasesAsJsonBytes() {
    CURL *curl = curl_easy_init();
    CurlDestroyer destroyer(curl); // destroy curl on exit
    if (!curl) {
      throw UpdateCheckerException("failed to initialize curl");
    }

    // Response body will be here
    std::vector<char> response;

    // Configure the request
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, USER_AGENT_HEADER);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, TIMEOUT_MS);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, RELEASES_API_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_received_bytes);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Create a multi-handle that uses non-blocking IO to make the request
    // interruptable
    CURLM *curlMulti = curl_multi_init();
    CurlMultiDestroyer multiDestroyer(curlMulti);
    if (!curlMulti) {
      throw UpdateCheckerException("failed to initialize curl multi handle");
    }
    curl_multi_add_handle(curlMulti, curl);

    int stillRunning = 1; // number of requests in progress
    while (stillRunning) {
      if (ShouldStop()) {
        throw UpdateCheckerException("Aborted");
      }
      // Wait for file descriptors to be ready
      int numFdsReady;
      CURLMcode waitCode
        = curl_multi_wait(curlMulti, nullptr, 0, 100, &numFdsReady);
      if (waitCode != CURLM_OK) {
        throw UpdateCheckerException(
          std::string("curl_multi_wait: ") + curl_multi_strerror(waitCode));
      }
      // Do actions with ready file descriptors
      CURLMcode performCode = curl_multi_perform(curlMulti, &stillRunning);
      if (performCode != CURLM_OK) {
        throw UpdateCheckerException(
          std::string("curl_multi_perform: ")
          + curl_multi_strerror(performCode));
      }
    }

    // Check the response status
    long httpCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCode != 200) {
      throw UpdateCheckerException(
        std::string("failed fetching the latest release: response code is ")
        + std::to_string(httpCode));
    }

    return response;
  }

  std::vector<GithubRelease> FetchLatestReleases() {
    std::vector<char> responseContent = FetchLatestReleasesAsJsonBytes();
    try {
      // JSON is valid YAML, use yaml-cpp so that we don't have to add another
      // project dependency
      YAML::Node response
        = YAML::Load(std::string(&responseContent[0], responseContent.size()));
      std::vector<GithubRelease> releases;
      for (const auto &node : response) {
        releases.push_back(GithubRelease::fromYaml(node));
      }
      return releases;
    } catch (const YAML::Exception &e) {
      throw UpdateCheckerException(
        std::string("error parsing github api response: ") + e.what());
    }
  }

  GOUpdateChecker::Result DoUpdateChecking() {
    try {
      // Fetch latest releases and filter out drafts and prereleases
      std::vector<GithubRelease> allReleases = FetchLatestReleases();
      allReleases.erase(
        std::remove_if(
          allReleases.begin(),
          allReleases.end(),
          [](const GithubRelease &r) { return r.draft || r.prerelease; }),
        allReleases.end());
      // Get the latest version
      if (allReleases.empty()) {
        return GOUpdateChecker::Result::error("no releases found");
      }
      const wxString &latestVersionStr = allReleases[0].name;
      // Compare version numbers
      GOVersion latestVersion = GOVersion(latestVersionStr);
      GOVersion currentVersion = GOVersion(APP_VERSION);
      bool updateAvailable = currentVersion.IsValid() && latestVersion.IsValid()
        && currentVersion < latestVersion;
      // Build the latest release metadata
      GOUpdateChecker::ReleaseMetadata metadata;
      metadata.version = latestVersionStr;
      if (updateAvailable) {
        // build changelog by concatenating bodies of releases
        for (const auto &release : allReleases) {
          GOVersion releaseVersion = GOVersion(release.name);
          if (releaseVersion.IsValid() && currentVersion < releaseVersion) {
            metadata.changelog
              += wxString::Format("%s\n%s\n", release.name, release.body);
          }
        }
      }
      wxLogDebug(
        "latest version: %s, update available: %s",
        latestVersionStr,
        updateAvailable ? "yes" : "no");
      return GOUpdateChecker::Result::success(metadata, updateAvailable);
    } catch (const UpdateCheckerException &e) {
      wxLogDebug("failed to check for updates: %s", e.what());
      return GOUpdateChecker::Result::error(e.what());
    }
  }
};

std::unique_ptr<GOThread> GOUpdateChecker::StartThread(
  GOUpdateChecker::CheckReason checkReason,
  wxEvtHandler *completionEventHandler) {
  auto thread = std::make_unique<CheckForUpdatesThread>(
    checkReason, completionEventHandler);
  thread->Start();
  return thread;
}

void GOUpdateChecker::OpenDownloadPageInBrowser() {
  wxLaunchDefaultBrowser(DOWNLOAD_URL);
}
