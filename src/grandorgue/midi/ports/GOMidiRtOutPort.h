/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRTOUTPORT_H
#define GOMIDIRTOUTPORT_H

#include "GOMidiOutPort.h"
#include "RtMidi.h"

class GOMidiRtOutPort : public GOMidiOutPort {
protected:
  RtMidi::Api m_api;
  RtMidiOut *m_port;

  void Close(bool isToFreePort);

  void SendData(std::vector<unsigned char> &msg);

public:
  GOMidiRtOutPort(
    GOMidi *midi,
    RtMidi::Api api,
    const wxString &deviceName,
    const wxString &fullName);
  ~GOMidiRtOutPort();

  virtual const wxString GetDefaultLogicalName() const;
  virtual const wxString GetDefaultRegEx() const;

  bool Open(unsigned id);
  void Close() { Close(true); }
};

#endif
