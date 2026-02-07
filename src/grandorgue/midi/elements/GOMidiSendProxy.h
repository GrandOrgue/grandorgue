/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDPROXY_H
#define GOMIDISENDPROXY_H

class GOMidiSystem;
class GOMidiEvent;
class GOMidiRecorder;

class GOMidiSendProxy {
  GOMidiSystem *p_midi = nullptr;
  GOMidiRecorder *p_MidiRecorder = nullptr;

public:
  void SetMidi(GOMidiSystem *pMidi, GOMidiRecorder *pMidiRecorder);
  void SendMidiMessage(const GOMidiEvent &e);
  void SendMidiRecorderMessage(GOMidiEvent &e);
};

#endif /* GOMIDISENDPROXY_H */
