/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSenderEventPatternList.h"

GOMidiSenderEventPatternList::GOMidiSenderEventPatternList(
  GOMidiSenderType type)
  : m_type(type), m_events() {}

GOMidiSenderEventPatternList::~GOMidiSenderEventPatternList() {}

GOMidiSenderType GOMidiSenderEventPatternList::GetType() const {
  return m_type;
}

unsigned GOMidiSenderEventPatternList::GetEventCount() const {
  return m_events.size();
}

GOMidiSenderEventPattern &GOMidiSenderEventPatternList::GetEvent(
  unsigned index) {
  return m_events[index];
}

unsigned GOMidiSenderEventPatternList::AddNewEvent() {
  GOMidiSenderEventPattern m = {0, MIDI_S_NONE, 1, 1, 0, 127};
  m_events.push_back(m);
  return m_events.size() - 1;
}

void GOMidiSenderEventPatternList::DeleteEvent(unsigned index) {
  m_events.erase(m_events.begin() + index);
}
