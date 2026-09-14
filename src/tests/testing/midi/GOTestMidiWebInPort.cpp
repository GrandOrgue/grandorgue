/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestMidiWebInPort.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define closesocket close
#endif

#include <chrono>
#include <cstring>
#include <format>

#include <wx/init.h>

#include "GOTestScope.h"
#include "midi/GOMidiSystem.h"
#include "midi/ports/GOMidiWebInPort.h"

const std::string GOTestMidiWebInPort::TEST_NAME = "GOTestMidiWebInPort";

// somewhere nobody else should be listening
static const unsigned TEST_PORT = 18790;

// the port posts received events to the midi system, which needs a wxApp
bool GOTestMidiWebInPort::setUp() { return GOTest::setUp() && wxInitialize(); }

bool GOTestMidiWebInPort::tearDown() {
  wxUninitialize();
  return GOTest::tearDown();
}

static std::vector<unsigned char> parse(const char *body, bool &outIsOk) {
  std::vector<unsigned char> msg;

  outIsOk = GOMidiWebInPort::parseMidiBody(body, msg);
  return msg;
}

void GOTestMidiWebInPort::TestAcceptsChannelMessages() {
  bool isOk;
  std::vector<unsigned char> msg = parse("9F 64 7F", isOk);

  GOAssert(isOk, "note on should be accepted");
  GOAssert(
    msg == std::vector<unsigned char>({0x9F, 0x64, 0x7F}),
    "note on should be parsed byte by byte");

  parse("CF 05", isOk);
  GOAssert(isOk, "two byte program change should be accepted");
  parse("b0 07 7f", isOk);
  GOAssert(isOk, "lower case hex should be accepted");
  parse("  8F\t64  00 ", isOk);
  GOAssert(isOk, "any whitespace should do as separator");
}

void GOTestMidiWebInPort::TestRejectsEverythingElse() {
  static const char *const BAD[] = {
    "",                  // nothing
    "F0 7D 47 4F 10 F7", // sysex, never from the network
    "F8",                // realtime
    "9F 80 7F",          // data byte with bit 7 set
    "9F 64",             // note needs 3 bytes
    "CF 05 00",          // program change needs 2
    "9F 64 7F 00",       // trailing byte
    "9F 64 7F 90 64 7F", // two messages at once
    "9F 1FF 7F",         // not a byte
    "9F xx 7F",          // not hex
    "64 7F",             // no status byte
  };

  for (const char *body : BAD) {
    bool isOk;

    parse(body, isOk);
    GOAssert(!isOk, std::format("'{}' should be rejected", body));
  }
}

static SOCKET connect_to(unsigned port) {
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons((unsigned short)port);
  if (
    sock != INVALID_SOCKET && connect(sock, (sockaddr *)&addr, sizeof(addr))) {
    closesocket(sock);
    sock = INVALID_SOCKET;
  }
  return sock;
}

void GOTestMidiWebInPort::TestServesAndClosesFast() {
  GOConfig config(TEST_NAME, "");
  GOMidiSystem midi(config);
  GOMidiWebInPort port(&midi, config);

  config.WebRemotePort(TEST_PORT);
  GOAssert(port.Open(1, 0), "the port should start listening");
  GOAssert(port.IsActive(), "and report itself active");

  // a few connections that never say anything, like a browser's spares
  SOCKET idle[3];

  for (SOCKET &s : idle) {
    s = connect_to(TEST_PORT);
    GOAssert(s != INVALID_SOCKET, "idle client should connect");
  }

  // and one real request in the middle of them
  const char request[]
    = "POST /midi HTTP/1.1\r\nHost: x\r\nContent-Length: 8\r\n\r\n9F 64 7F";
  SOCKET req = connect_to(TEST_PORT);
  char reply[64] = {0};

  GOAssert(req != INVALID_SOCKET, "request client should connect");
  send(req, request, sizeof(request) - 1, 0);
  recv(req, reply, sizeof(reply) - 1, 0);
  GOAssert(
    std::string(reply).starts_with("HTTP/1.1 204"),
    std::format("a valid note should get a 204, got '{}'", reply));
  closesocket(req);

  // Close() must not sit around waiting for the idle ones to time out
  const auto start = std::chrono::steady_clock::now();

  port.Close();

  const auto tookMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start)
                        .count();

  GOAssert(!port.IsActive(), "the port should be inactive after Close()");
  GOAssert(
    tookMs < 1000, std::format("Close() should be quick, took {} ms", tookMs));
  for (SOCKET s : idle)
    closesocket(s);
  GOAssert(
    connect_to(TEST_PORT) == INVALID_SOCKET,
    "nothing should be listening any more");
}

void GOTestMidiWebInPort::run() {
  GO_RUN_TEST(TestAcceptsChannelMessages())
  GO_RUN_TEST(TestRejectsEverythingElse())
  GO_RUN_TEST(TestServesAndClosesFast())
}
