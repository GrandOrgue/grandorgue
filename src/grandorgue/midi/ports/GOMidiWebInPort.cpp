/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiWebInPort.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define SHUT_RDWR SD_BOTH
#else
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

// make the posix side look like winsock so the rest of the file is ifdef-free
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define closesocket close
#endif

// A peer that went away must not kill us with SIGPIPE. Linux has a per-send
// flag for that, macOS a socket option, windows never raises it.
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <system_error>
#include <thread>
#include <vector>

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfig.h"
#include "midi/GOMidiSystem.h"

// generated from resource/web-remote.html at build time
extern const char *const GOMidiWebRemotePage;

const wxString GOMidiWebInPort::PORT_NAME = wxT("Web");
const wxString GOMidiWebInPort::DEVICE_NAME = wxT("Web Remote");

// How long a single recv()/send() may block, and how long a whole request may
// take. Both bound how long Close() may wait for a client thread.
static const unsigned SOCKET_TIMEOUT_MS = 2000;
static const std::chrono::seconds REQUEST_DEADLINE(5);
static const size_t MAX_REQUEST_SIZE = 16 * 1024;
// Plenty for one phone and a browser's spare connections. Anything above
// smells like a flood and is dropped on the floor.
static const size_t MAX_CLIENTS = 8;

using Clock = std::chrono::steady_clock;

GOMidiWebInPort::GOMidiWebInPort(GOMidiSystem *midi, GOConfig &config)
  : GOMidiInPort(midi, PORT_NAME, wxEmptyString, DEVICE_NAME, DEVICE_NAME),
    r_config(config) {
#ifdef _WIN32
  WSADATA wsa;

  WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

GOMidiWebInPort::~GOMidiWebInPort() {
  Close();
#ifdef _WIN32
  WSACleanup();
#endif
}

static void set_timeouts(SOCKET sock, unsigned ms) {
#ifdef _WIN32
  DWORD tv = ms;
#else
  timeval tv;

  tv.tv_sec = ms / 1000;
  tv.tv_usec = (ms % 1000) * 1000;
#endif
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));
#ifdef SO_NOSIGPIPE
  int one = 1;

  setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&one, sizeof(one));
#endif
}

static void send_all(SOCKET sock, const std::string &data) {
  size_t sent = 0;

  while (sent < data.size()) {
    int n = send(
      sock, data.data() + sent, (int)(data.size() - sent), MSG_NOSIGNAL);

    if (n <= 0)
      break;
    sent += n;
  }
}

static void send_response(
  SOCKET sock,
  const char *status,
  const char *contentType,
  const std::string &body) {
  std::string head = std::string("HTTP/1.1 ") + status + "\r\n";

  head += std::string("Content-Type: ") + contentType + "\r\n";
  head += "Content-Length: " + std::to_string(body.size()) + "\r\n";
  head += "Cache-Control: no-store\r\n";
  head += "Connection: close\r\n\r\n";
  send_all(sock, head + body);
}

// recv() that also gives up once the whole request has taken too long, so a
// client trickling one byte at a time can't keep a thread forever
static int recv_before(
  SOCKET sock, char *buf, size_t len, Clock::time_point deadline) {
  return Clock::now() < deadline ? recv(sock, buf, (int)len, 0) : -1;
}

// Reads a whole request. We are not a real web server: just enough HTTP to
// get the request line and, for POST, the body.
static bool read_request(
  SOCKET sock,
  std::string &outMethod,
  std::string &outPath,
  std::string &outBody) {
  const Clock::time_point deadline = Clock::now() + REQUEST_DEADLINE;
  std::string buf;
  size_t headEnd = std::string::npos;
  char chunk[1024];
  bool isOk = false;

  // headers first
  while (headEnd == std::string::npos && buf.size() < MAX_REQUEST_SIZE) {
    int n = recv_before(sock, chunk, sizeof(chunk), deadline);

    if (n <= 0)
      break;
    buf.append(chunk, n);
    headEnd = buf.find("\r\n\r\n");
  }

  if (headEnd != std::string::npos) {
    const std::string head = buf.substr(0, headEnd);
    const size_t sp1 = head.find(' ');
    const size_t sp2 = sp1 == std::string::npos ? sp1 : head.find(' ', sp1 + 1);

    if (sp2 != std::string::npos) {
      outMethod = head.substr(0, sp1);
      outPath = head.substr(sp1 + 1, sp2 - sp1 - 1);

      // Content-Length is the only header we care about. Lowercase copy
      // because browsers don't agree on the casing, and look for it at the
      // start of a line so a value or the path can't fake it.
      std::string lower = head;
      size_t contentLength = 0;

      for (char &c : lower)
        c = tolower(c);

      const size_t clPos = lower.find("\ncontent-length:");

      if (clPos != std::string::npos)
        contentLength = strtoul(lower.c_str() + clPos + 16, NULL, 10);

      if (contentLength <= MAX_REQUEST_SIZE) {
        outBody = buf.substr(headEnd + 4);
        while (outBody.size() < contentLength) {
          int n = recv_before(sock, chunk, sizeof(chunk), deadline);

          if (n <= 0)
            break;
          outBody.append(chunk, n);
        }
        isOk = outBody.size() >= contentLength;
      }
    }
  }
  return isOk;
}

// Only channel messages get through: no SysEx from the network, GrandOrgue
// reacts to some of it in ways nobody should be able to trigger from a browser.
bool GOMidiWebInPort::parseMidiBody(
  const std::string &body, std::vector<unsigned char> &outMsg) {
  std::vector<unsigned char> &msg = outMsg;
  const char *p = body.c_str();
  bool isOk = true;

  msg.clear();
  while (*p && isOk && msg.size() <= 3) {
    char *end;
    unsigned long v = strtoul(p, &end, 16);

    if (end == p) {
      // not a hex digit. Skip separators, anything else is garbage
      if (isspace((unsigned char)*p))
        p++;
      else
        isOk = false;
    } else if (v > 0xFF) {
      isOk = false;
    } else {
      msg.push_back((unsigned char)v);
      p = end;
    }
  }
  if (isOk && !msg.empty()) {
    const unsigned char status = msg[0];
    // program change and channel pressure are the two 2-byte ones
    const size_t expected
      = (status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0 ? 2 : 3;

    isOk = status >= 0x80 && status < 0xF0 && msg.size() == expected
      && msg[1] < 0x80 && (expected == 2 || msg[2] < 0x80);
  } else {
    isOk = false;
  }
  return isOk;
}

bool GOMidiWebInPort::HandleMidiPost(const std::string &body) {
  std::vector<unsigned char> msg;
  const bool isOk = parseMidiBody(body, msg);

  if (isOk) {
    std::lock_guard<std::mutex> lock(m_ReceiveMutex);

    Receive(msg);
  }
  return isOk;
}

bool GOMidiWebInPort::RegisterClient(intptr_t sock) {
  std::lock_guard<std::mutex> lock(m_ClientsMutex);
  const bool isAccepted = m_Clients.size() < MAX_CLIENTS;

  if (isAccepted)
    m_Clients.insert(sock);
  return isAccepted;
}

void GOMidiWebInPort::UnregisterClient(intptr_t sock) {
  std::lock_guard<std::mutex> lock(m_ClientsMutex);

  m_Clients.erase(sock);
}

void GOMidiWebInPort::ServeClient(intptr_t sockValue) {
  const SOCKET sock = (SOCKET)sockValue;
  std::string method, path, body;

  set_timeouts(sock, SOCKET_TIMEOUT_MS);
  if (!read_request(sock, method, path, body))
    send_response(sock, "400 Bad Request", "text/plain", "bad request");
  else if (method == "GET" && (path == "/" || path == "/index.html"))
    send_response(
      sock, "200 OK", "text/html; charset=utf-8", GOMidiWebRemotePage);
  else if (method == "POST" && path == "/midi")
    send_response(
      sock,
      HandleMidiPost(body) ? "204 No Content" : "400 Bad Request",
      "text/plain",
      "");
  else
    send_response(sock, "404 Not Found", "text/plain", "not found");
  // unregister first: once we're out of the set Close() may return and the
  // object may be gone, so nothing of `this` is touched after this line
  UnregisterClient(sockValue);
  closesocket(sock);
}

void GOMidiWebInPort::Entry() {
  const SOCKET listenSock = (SOCKET)m_ListenSock;

  // Each connection gets its own short-lived thread. Browsers like to open
  // spare connections that never send anything, and with a single thread one
  // of those would hold every button press until the receive timeout. The
  // select timeout is what lets Close() stop us.
  while (!ShouldStop()) {
    fd_set fds;
    timeval tv;

    FD_ZERO(&fds);
    FD_SET(listenSock, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;

    if (select((int)listenSock + 1, &fds, NULL, NULL, &tv) > 0) {
      SOCKET client = accept(listenSock, NULL, NULL);

      if (client != INVALID_SOCKET) {
        bool isServed = false;

        if (RegisterClient(client))
          try {
            std::thread([this, client]() { ServeClient(client); }).detach();
            isServed = true;
          } catch (const std::system_error &) {
            // out of threads. Drop this one, the page will just retry
            UnregisterClient(client);
          }
        if (!isServed)
          closesocket(client);
      }
    }
  }
}

bool GOMidiWebInPort::Open(unsigned id, int channelShift) {
  const unsigned port = r_config.WebRemotePort();

  Close();

  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock != INVALID_SOCKET) {
    sockaddr_in addr;

#ifndef _WIN32
    // so a quick close/reopen doesn't fail with "address in use". Not on
    // windows, where the same flag lets another program grab our port.
    int one = 1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
#endif
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);

    if (
      bind(sock, (sockaddr *)&addr, sizeof(addr)) == 0
      && listen(sock, 8) == 0) {
      m_ListenSock = sock;
      // The default button mappings are bound to DEVICE_NAME, so that is the
      // id we report, whatever the device got renamed to in the settings.
      // Must be in place before the first request can come in.
      GOMidiInPort::Open(
        m_midi->GetMidiMap().EnsureLogicalName(DEVICE_NAME), channelShift);
      m_IsActive = true;
      Start();
    } else {
      closesocket(sock);
    }
  }
  if (!m_IsActive)
    wxLogError(
      _("Web Remote: cannot listen on port %u. Is it used by something else?"),
      port);
  return m_IsActive;
}

void GOMidiWebInPort::Close() {
  if (m_ListenSock != -1) {
    Stop(); // waits for the accept loop to notice
    closesocket((SOCKET)m_ListenSock);
    m_ListenSock = -1;

    // Cut the clients short instead of waiting for their timeouts: shutdown
    // makes their recv()/send() return right away and the thread finishes.
    {
      std::lock_guard<std::mutex> lock(m_ClientsMutex);

      for (intptr_t sock : m_Clients)
        shutdown((SOCKET)sock, SHUT_RDWR);
    }
    for (bool isBusy = true; isBusy;) {
      {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);

        isBusy = !m_Clients.empty();
      }
      if (isBusy)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  m_IsActive = false;
  GOMidiPort::Close();
}
