/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIRTOUTPORT_H
#define GORGUEMIDIRTOUTPORT_H

#include "GOrgueMidiOutPort.h"
#include "RtMidi.h"

class GOrgueMidiRtOutPort : public GOrgueMidiOutPort
{
protected:
  RtMidi::Api m_api;
  RtMidiOut* m_port;

  void Close(bool isToFreePort);

  void SendData(std::vector<unsigned char>& msg);

public:
  GOrgueMidiRtOutPort(GOrgueMidi* midi, wxString prefix, wxString name, RtMidi::Api api);
  ~GOrgueMidiRtOutPort();

  bool Open();
  void Close() { Close(true); }
};

#endif
