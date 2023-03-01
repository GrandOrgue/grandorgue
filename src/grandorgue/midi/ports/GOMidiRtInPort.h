/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRTINPORT_H
#define GOMIDIRTINPORT_H

#include "GOMidiInPort.h"
#include "RtMidi.h"

class GOMidiRtInPort : public GOMidiInPort {
protected:
  const RtMidi::Api m_api;
  RtMidiIn *m_port;

  void Close(bool isToFreePort);

  static void MIDICallback(
    double timeStamp, std::vector<unsigned char> *msg, void *userData);

public:
  GOMidiRtInPort(
    GOMidi *midi,
    RtMidi::Api api,
    const wxString &deviceName,
    const wxString &fullName);
  ~GOMidiRtInPort();

  virtual const wxString GetDefaultLogicalName() const;
  virtual const wxString GetDefaultRegEx() const;

  bool Open(unsigned id, int channel_shift = 0);
  void Close() { Close(true); }
};

#endif
