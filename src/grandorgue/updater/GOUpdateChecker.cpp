/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOUpdateChecker.h"

#include <iostream>
#include <thread>

#include <curl/curl.h>
#include <wx/regex.h>
#include <wx/string.h>
#include <yaml-cpp/yaml.h>

#include "yaml/go-wx-yaml.h"

#include "go_defs.h"
#include "settings.h"
#include "threading/GOThread.h"

wxDEFINE_EVENT(UPDATE_CHECKING_COMPLETION, GOUpdateChecker::CompletionEvent);

class GOVersion {
public:
  explicit GOVersion(const wxString &str) {
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
  std::array<int, 4> m_components{};
  bool m_valid{};
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

static size_t handle_received_bytes(
  void *contents, size_t size, size_t nmemb, std::vector<char> *dst) {
  size_t bytesReceived = size * nmemb;
  char *contentsAsChars = static_cast<char *>(contents);
  dst->reserve(dst->size() + bytesReceived);
  dst->insert(dst->end(), contentsAsChars, contentsAsChars + bytesReceived);
  return bytesReceived;
}

static std::vector<char> fetch_latest_release_as_json_bytes() {
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
  curl_easy_setopt(curl, CURLOPT_URL, LATEST_RELEASE_API_URL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_received_bytes);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  // Perform the request
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    throw UpdateCheckerException(
      std::string("failed fetching the latest release: ")
      + curl_easy_strerror(res));
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

static GOUpdateChecker::ReleaseMetadata fetch_latest_release() {
  std::vector<char> responseContent = fetch_latest_release_as_json_bytes();
  try {
    // JSON is valid YAML, use yaml-cpp so that we don't have to add another
    // project dependency
    YAML::Node response
      = YAML::Load(std::string(&responseContent[0], responseContent.size()));
    GOUpdateChecker::ReleaseMetadata metadata;
    metadata.version = response["name"].as<wxString>();
    metadata.changelog = response["body"].as<wxString>();
    return metadata;
  } catch (const YAML::Exception &e) {
    throw UpdateCheckerException(
      std::string("error parsing github api response: ") + e.what());
  }
}

class CheckForUpdatesThread : public GOThread {
public:
  explicit CheckForUpdatesThread(wxEvtHandler *completionEventHandler)
    : m_CompletionEventHandler(completionEventHandler) {}

protected:
  void Entry() override {
    try {
      GOUpdateChecker::Result result;
      // Fetch latest release
      result.latestRelease = fetch_latest_release();
      // Compare versions
      GOVersion releaseVersion = GOVersion(result.latestRelease.version);
      GOVersion currentVersion = GOVersion(APP_VERSION);
      result.updateAvailable = currentVersion.IsValid()
        && releaseVersion.IsValid() && currentVersion < releaseVersion;
      // Fire completion event
      m_CompletionEventHandler->QueueEvent(new GOUpdateChecker::CompletionEvent(
        UPDATE_CHECKING_COMPLETION, result));
      // Log results
      wxLogDebug(
        "latest version: %s, update available: %s",
        result.latestRelease.version,
        result.updateAvailable ? "yes" : "no");
    } catch (UpdateCheckerException &e) {
      wxLogDebug("failed to check for updates: %s", e.what());
    }
  }

private:
  wxEvtHandler *m_CompletionEventHandler;
};

std::unique_ptr<GOThread> GOUpdateChecker::StartThread(
  wxEvtHandler *completionEventHandler) {
  std::unique_ptr<GOThread> thread(
    new CheckForUpdatesThread(completionEventHandler));
  thread->Start();
  return thread;
}
