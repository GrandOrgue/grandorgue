/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSendProxy.h"

#include "midi/GOMidiRecorder.h"
#include "midi/GOMidiSystem.h"

GOMidiSendProxy::StateRestorer::StateRestorer(GOMidiSendProxy &midiSendProxy)
  : r_MidiSendProxy(midiSendProxy),
    m_IsToSendMidi(midiSendProxy.IsToSendMidi()),
    m_IsToRecordMidi(midiSendProxy.IsToRecordMidi()) {}

GOMidiSendProxy::StateRestorer::~StateRestorer() {
  r_MidiSendProxy.SetToSendMidi(m_IsToSendMidi);
  r_MidiSendProxy.SetToRecordMidi(m_IsToRecordMidi);
}

void GOMidiSendProxy::SetMidi(
  GOMidiSystem *pMidi, GOMidiRecorder *pMidiRecorder) {
  p_midi = pMidi;
  p_MidiRecorder = pMidiRecorder;
}

void GOMidiSendProxy::SendMidiMessage(const GOMidiEvent &e) {
  if (p_midi && m_IsToSendMidi)
    p_midi->Send(e);
}

void GOMidiSendProxy::SendMidiRecorderMessage(GOMidiEvent &e) {
  if (p_MidiRecorder && m_IsToRecordMidi)
    p_MidiRecorder->SendMidiRecorderMessage(e);
}
