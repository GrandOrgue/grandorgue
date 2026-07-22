/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiPlayerContent.h"

#include <algorithm>
#include <cassert>
#include <set>

#include "midi/events/GOMidiEvent.h"

#include "GOMidiInputMerger.h"
#include "GOMidiMap.h"

GOMidiPlayerContent::GOMidiPlayerContent() : m_Events(), m_Pos(0) { Clear(); }

GOMidiPlayerContent::~GOMidiPlayerContent() {}

void GOMidiPlayerContent::Clear() {
  m_Events.clear();
  Reset();
}

void GOMidiPlayerContent::Reset() { m_Pos = 0; }

bool GOMidiPlayerContent::IsLoaded() { return m_Events.size() > 0; }

bool GOMidiPlayerContent::hasNativeHeader(
  const std::vector<GOMidiEvent> &events) {
  return !events.empty()
    && events[0].GetMidiType() == GOMidiEvent::MIDI_SYSEX_GO_CLEAR;
}

bool GOMidiPlayerContent::isMidiInputNumberMappingUsable(
  const std::vector<int> &midiInputNumbers) {
  const bool isEveryNumberInRange = std::all_of(
    midiInputNumbers.begin(), midiInputNumbers.end(), [](int midiInputNumber) {
      return midiInputNumber >= 1 && midiInputNumber <= 16;
    });
  const std::set<int> distinctNumbers(
    midiInputNumbers.begin(), midiInputNumbers.end());
  const bool areAllNumbersDistinct
    = distinctNumbers.size() == midiInputNumbers.size();

  return isEveryNumberInRange && areAllNumbersDistinct;
}

std::vector<unsigned> GOMidiPlayerContent::computeManualChannels(
  bool hasPedal,
  const std::vector<int> &midiInputNumbers,
  GOConfig::MidiFileChannelMapping mappingMode) {
  std::vector<unsigned> channels;

  if (
    mappingMode == GOConfig::MIDI_PLAY_CHANNELS_USE_INPUT_NUMBER
    && isMidiInputNumberMappingUsable(midiInputNumbers))
    for (int midiInputNumber : midiInputNumbers)
      channels.push_back((unsigned)midiInputNumber);
  else {
    const bool isPedalFirst
      = mappingMode == GOConfig::MIDI_PLAY_CHANNELS_PEDAL_FIRST;
    const unsigned nManuals = midiInputNumbers.size() - (hasPedal ? 1 : 0);
    const unsigned pedalChannel = isPedalFirst ? 1 : nManuals + 1;
    unsigned nextManualChannel = isPedalFirst && hasPedal ? 2 : 1;

    if (hasPedal)
      channels.push_back(pedalChannel);
    for (unsigned manualI = nManuals; manualI > 0; manualI--)
      channels.push_back(nextManualChannel++);
  }

  assert(channels.size() == midiInputNumbers.size());

  return channels;
}

void GOMidiPlayerContent::SetupManual(
  GOMidiMap &map, unsigned channel, wxString ID) {
  GOMidiEvent e;
  e.SetTime(0);

  e.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_SETUP);
  e.SetKey(map.EnsureRecorderElementName(ID));
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
  const std::vector<GOMidiEvent> &events,
  GOMidiMap &map,
  bool hasPedal,
  const std::vector<int> &midiInputNumbers,
  GOConfig::MidiFileChannelMapping mappingMode) {
  Clear();

  GOMidiMerger merger;
  merger.Clear();
  if (!events.empty() && !hasNativeHeader(events)) {
    GOMidiEvent e;
    e.SetTime(0);
    e.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_CLEAR);
    e.SetChannel(0);
    m_Events.push_back(e);

    const std::vector<unsigned> channels
      = computeManualChannels(hasPedal, midiInputNumbers, mappingMode);
    const unsigned manualIndexOffset = hasPedal ? 0 : 1;

    for (unsigned n = channels.size(), i = 0; i < n; i++)
      SetupManual(
        map, channels[i], wxString::Format(wxT("M%d"), i + manualIndexOffset));
  }
  for (GOMidiEvent e : events)
    if (merger.Process(e))
      m_Events.push_back(e);

  return true;
}

const GOMidiEvent &GOMidiPlayerContent::GetCurrentEvent() {
  return m_Events[m_Pos];
}

bool GOMidiPlayerContent::Next() {
  m_Pos++;
  return m_Pos < m_Events.size();
}
