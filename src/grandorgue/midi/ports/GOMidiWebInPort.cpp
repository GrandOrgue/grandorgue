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

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfig.h"

// generated from resource/web-remote.html at build time
extern const char *const GOMidiWebRemotePage;

const wxString GOMidiWebInPort::PORT_NAME = wxT("Web");
const wxString GOMidiWebInPort::DEVICE_NAME = wxT("Web Remote");

// how long a client may keep us waiting for its request. Also bounds how long
// Close() may block while a request is in flight.
static const unsigned RECV_TIMEOUT_MS = 2000;
static const size_t MAX_REQUEST_SIZE = 16 * 1024;

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

static void set_recv_timeout(SOCKET sock, unsigned ms) {
#ifdef _WIN32
  DWORD tv = ms;
#else
  timeval tv;

  tv.tv_sec = ms / 1000;
  tv.tv_usec = (ms % 1000) * 1000;
#endif
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
}

static void send_all(SOCKET sock, const std::string &data) {
  size_t sent = 0;

  while (sent < data.size()) {
    int n = send(sock, data.data() + sent, (int)(data.size() - sent), 0);

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

// Reads a whole request. We are not a real web server: just enough HTTP to
// get the request line and, for POST, the body.
static bool read_request(
  SOCKET sock,
  std::string &outMethod,
  std::string &outPath,
  std::string &outBody) {
  std::string buf;
  size_t headEnd = std::string::npos;
  char chunk[1024];
  bool isOk = false;

  // headers first
  while (headEnd == std::string::npos && buf.size() < MAX_REQUEST_SIZE) {
    int n = recv(sock, chunk, sizeof(chunk), 0);

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
      // because browsers don't agree on the casing.
      std::string lower = head;
      size_t contentLength = 0;

      for (char &c : lower)
        c = tolower(c);

      const size_t clPos = lower.find("content-length:");

      if (clPos != std::string::npos)
        contentLength = strtoul(lower.c_str() + clPos + 15, NULL, 10);

      if (contentLength <= MAX_REQUEST_SIZE) {
        outBody = buf.substr(headEnd + 4);
        while (outBody.size() < contentLength) {
          int n = recv(sock, chunk, sizeof(chunk), 0);

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

// body is hex bytes separated by whitespace, e.g. "90 3C 7F"
bool GOMidiWebInPort::HandleMidiPost(const std::string &body) {
  std::vector<unsigned char> msg;
  const char *p = body.c_str();
  bool isOk = true;

  while (*p && isOk) {
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
  isOk = isOk && !msg.empty() && msg.size() <= 64;
  if (isOk)
    Receive(msg);
  return isOk;
}

void GOMidiWebInPort::ServeClient(intptr_t sockValue) {
  const SOCKET sock = (SOCKET)sockValue;
  std::string method, path, body;

  set_recv_timeout(sock, RECV_TIMEOUT_MS);
  if (read_request(sock, method, path, body)) {
    if (method == "GET" && (path == "/" || path == "/index.html"))
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
  }
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
        m_NClients++;
        std::thread([this, client]() {
          ServeClient(client);
          m_NClients--;
        }).detach();
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
    int one = 1;

    // so a quick close/reopen doesn't fail with "address in use"
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);

    if (
      bind(sock, (sockaddr *)&addr, sizeof(addr)) == 0
      && listen(sock, 8) == 0) {
      m_ListenSock = sock;
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
  return GOMidiInPort::Open(id, channelShift);
}

void GOMidiWebInPort::Close() {
  if (m_ListenSock != -1) {
    Stop(); // waits for the accept loop to notice
    closesocket((SOCKET)m_ListenSock);
    m_ListenSock = -1;
    // client threads are bounded by RECV_TIMEOUT_MS, so this is short
    while (m_NClients)
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  m_IsActive = false;
  GOMidiPort::Close();
}
