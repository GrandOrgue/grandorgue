/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiPlayerContent.h"

#include "GOMidiInputMerger.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiFileReader.h"
#include "midi/GOMidiMap.h"

GOMidiPlayerContent::GOMidiPlayerContent() : m_Events(), m_Pos(0) { Clear(); }

GOMidiPlayerContent::~GOMidiPlayerContent() {}

void GOMidiPlayerContent::Clear() {
  m_Events.clear();
  Reset();
}

void GOMidiPlayerContent::Reset() { m_Pos = 0; }

bool GOMidiPlayerContent::IsLoaded() { return m_Events.size() > 0; }

void GOMidiPlayerContent::ReadFileContent(
  GOMidiFileReader &reader, std::vector<GOMidiEvent> &events) {
  unsigned pos = 0;
  GOMidiEvent e;
  while (reader.ReadEvent(e)) {
    while (pos > 0 && events[pos - 1].GetTime() > e.GetTime()) {
      pos--;
    }
    while (pos < events.size() && events[pos].GetTime() < e.GetTime()) {
      pos++;
    }
    if (pos <= events.size())
      events.insert(events.begin() + pos, e);
    else
      events.push_back(e);
    pos++;
  }
}

void GOMidiPlayerContent::SetupManual(
  GOMidiMap &map, unsigned channel, wxString ID) {
  GOMidiEvent e;
  e.SetTime(0);

  e.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_SETUP);
  e.SetKey(map.GetElementByString(ID));
  e.SetChannel(channel);
  e.SetValue(0);
  m_Events.push_back(e);

  e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
  e.SetChannel(channel);
  e.SetKey(MIDI_CTRL_NOTES_OFF);
  e.SetValue(0);
  m_Events.push_back(e);
}

bool GOMidiPlayerContent::Load(
  GOMidiFileReader &reader, GOMidiMap &map, unsigned manuals, bool pedal) {
  Clear();
  std::vector<GOMidiEvent> events;
  ReadFileContent(reader, events);

  GOMidiMerger merger;
  merger.Clear();
  if (
    events.size()
    && events[0].GetMidiType() != GOMidiEvent::MIDI_SYSEX_GO_CLEAR) {
    GOMidiEvent e;
    e.SetTime(0);
    e.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_CLEAR);
    e.SetChannel(0);
    m_Events.push_back(e);

    for (unsigned i = 1; i <= manuals; i++)
      SetupManual(map, i, wxString::Format(wxT("M%d"), i));
    if (pedal)
      SetupManual(map, manuals + 1, wxString::Format(wxT("M%d"), 0));
  }
  for (unsigned i = 0; i < events.size(); i++)
    if (merger.Process(events[i]))
      m_Events.push_back(events[i]);

  return true;
}

const GOMidiEvent &GOMidiPlayerContent::GetCurrentEvent() {
  return m_Events[m_Pos];
}

bool GOMidiPlayerContent::Next() {
  m_Pos++;
  return m_Pos < m_Events.size();
}
