/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIRTINPORT_H
#define GORGUEMIDIRTINPORT_H

#include "GOrgueMidiInPort.h"
#include "RtMidi.h"

class GOrgueMidiRtInPort : public GOrgueMidiInPort
{
protected:
  const RtMidi::Api m_api;
  RtMidiIn* m_port;

  void Close(bool isToFreePort);

  static void MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData);

public:
  GOrgueMidiRtInPort(GOrgueMidi* midi, wxString prefix, wxString name, RtMidi::Api api);
  ~GOrgueMidiRtInPort();

  bool Open(int channel_shift = 0);
  void Close() { Close(true); }
};

#endif
