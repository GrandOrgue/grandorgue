/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiOutputMerger.h"

#include "GORodgers.h"

GOMidiOutputMerger::GOMidiOutputMerger() : m_HWState(), m_RodgersState() {}

void GOMidiOutputMerger::Clear() {
  m_HWState.clear();
  m_RodgersState.clear();
}

bool GOMidiOutputMerger::Process(GOMidiEvent &e) {
  if (
    e.GetMidiType() == GOMidiEvent::MIDI_SYSEX_HW_STRING
    || e.GetMidiType() == GOMidiEvent::MIDI_SYSEX_HW_LCD) {
    unsigned item = 0;
    while (item < m_HWState.size()) {
      if (
        m_HWState[item].type == e.GetMidiType()
        && m_HWState[item].key == e.GetKey())
        break;
      item++;
    }
    if (item >= m_HWState.size()) {
      GOMidiOutputMergerHWState s;
      s.type = e.GetMidiType();
      s.key = e.GetKey();
      if (e.GetMidiType() == GOMidiEvent::MIDI_SYSEX_HW_STRING)
        s.content = wxString(wxT(' '), 16);
      else
        s.content = wxString(wxT(' '), 32);
      m_HWState.push_back(s);
    }
    GOMidiOutputMergerHWState &s = m_HWState[item];
    for (unsigned i = e.GetValue(), j = 0;
         i < s.content.length() && j < e.GetString().length();
         i++, j++)
      s.content[i] = e.GetString()[j];
    e.SetString(s.content);
  }
  if (e.GetMidiType() == GOMidiEvent::MIDI_SYSEX_RODGERS_STOP_CHANGE) {
    if ((unsigned)e.GetChannel() >= m_RodgersState.size())
      m_RodgersState.resize(e.GetChannel() + 1);
    std::vector<uint8_t> &data = m_RodgersState[e.GetChannel()];
    unsigned offset = GORodgersSetBit(e.GetKey(), e.GetValue(), data);
    std::vector<uint8_t> out(1);
    out[0] = data[offset];
    e.SetKey(offset);
    e.SetData(out);
  }
  return true;
}
