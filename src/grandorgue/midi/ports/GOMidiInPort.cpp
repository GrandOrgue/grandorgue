/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiInPort.h"

#include <wx/stopwatch.h>

#include "midi/GOMidi.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiMap.h"

GOMidiInPort::GOMidiInPort(
  GOMidi *midi,
  const wxString &portName,
  const wxString &apiName,
  const wxString &deviceName,
  const wxString &fullName)
  : GOMidiPort(midi, portName, apiName, deviceName, fullName),
    m_merger(),
    m_ChannelShift(0) {}

GOMidiInPort::~GOMidiInPort() {}

void GOMidiInPort::Receive(const std::vector<unsigned char> msg) {
  if (!IsActive())
    return;

  GOMidiEvent e;
  e.FromMidi(msg, m_midi->GetMidiMap());
  if (e.GetMidiType() == GOMidiEvent::MIDI_NONE)
    return;
  e.SetDevice(GetID());
  e.SetTime(wxGetLocalTimeMillis());

  if (!m_merger.Process(e))
    return;

  /* Compat stuff */
  if (e.GetChannel() != -1)
    e.SetChannel(((e.GetChannel() - 1 + m_ChannelShift) & 0x0F) + 1);

  m_midi->Recv(e);
}

bool GOMidiInPort::Open(unsigned id, int channel_shift) {
  GOMidiPort::Open(id);
  m_ChannelShift = channel_shift;
  m_merger.Clear();

  return m_IsActive;
}

const wxString GOMidiInPort::GetMyNativePortName() const {
  return wxT("GrandOrgue Input");
}
