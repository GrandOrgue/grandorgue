/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSendProxy.h"

#include "GOMidi.h"
#include "GOMidiRecorder.h"

void GOMidiSendProxy::SetMidi(GOMidi *pMidi, GOMidiRecorder *pMidiRecorder) {
  p_midi = pMidi;
  p_MidiRecorder = pMidiRecorder;
}

void GOMidiSendProxy::SendMidiMessage(const GOMidiEvent &e) {
  if (p_midi)
    p_midi->Send(e);
}

void GOMidiSendProxy::SendMidiRecorderMessage(GOMidiEvent &e) {
  if (p_MidiRecorder)
    p_MidiRecorder->SendMidiRecorderMessage(e);
}
