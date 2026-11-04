/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIWEBINPORT_H
#define GOMIDIWEBINPORT_H

#include <cstdint>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "GOMidiInPort.h"
#include "threading/GOThread.h"

class GOConfig;

/**
 * A virtual MIDI input device backed by a tiny HTTP server.
 *
 * The page it serves (resource/web-remote.html) shows a grid of buttons that
 * can be opened from a phone or tablet on the same network. Pressing a button
 * posts a MIDI channel message to /midi and we feed it into GrandOrgue exactly
 * as if it came from a hardware controller, so the usual "Listen for event"
 * workflow works for mapping the buttons to pistons, stops, Set, etc.
 *
 * There is no authentication: anyone on the network can push the buttons.
 * Only short channel messages are accepted, nothing that could reconfigure
 * GrandOrgue.
 */
class GOMidiWebInPort : public GOMidiInPort, private GOThread {
private:
  GOConfig &r_config;
  // the listening socket; -1 when not listening (also matches INVALID_SOCKET
  // on windows once cast). Kept as intptr_t to avoid winsock in the header.
  intptr_t m_ListenSock = -1;
  // connections being served right now, so Close() can cut them short and
  // wait for their threads
  std::mutex m_ClientsMutex;
  std::set<intptr_t> m_Clients;
  // Receive() is not thread safe and every connection has its own thread
  std::mutex m_ReceiveMutex;

  void Entry() override;
  void ServeClient(intptr_t sock);
  bool HandleMidiPost(const std::string &body);
  bool RegisterClient(intptr_t sock);
  void UnregisterClient(intptr_t sock);

public:
  static const wxString PORT_NAME;
  static const wxString DEVICE_NAME;

  GOMidiWebInPort(GOMidiSystem *midi, GOConfig &config);
  ~GOMidiWebInPort();

  /**
   * Parse a /midi body: hex bytes separated by whitespace, e.g. "9F 64 7F".
   * Only a single, well formed channel message is accepted.
   * @return whether outMsg holds a message worth forwarding
   */
  static bool parseMidiBody(
    const std::string &body, std::vector<unsigned char> &outMsg);

  bool IsToAutoEnable() const override { return false; }
  bool Open(unsigned id, int channelShift) override;
  void Close() override;
};

#endif
