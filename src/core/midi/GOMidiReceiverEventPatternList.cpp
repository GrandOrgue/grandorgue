/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiReceiverEventPatternList.h"

GOMidiReceiverEventPatternList::GOMidiReceiverEventPatternList(
  GOMidiReceiverType type)
  : m_type(type), m_events(0) {}

GOMidiReceiverEventPatternList::~GOMidiReceiverEventPatternList() {}

GOMidiReceiverType GOMidiReceiverEventPatternList::GetType() const {
  return m_type;
}

unsigned GOMidiReceiverEventPatternList::GetEventCount() const {
  return m_events.size();
}

GOMidiReceiverEventPattern &GOMidiReceiverEventPatternList::GetEvent(
  unsigned index) {
  return m_events[index];
}

unsigned GOMidiReceiverEventPatternList::AddNewEvent() {
  GOMidiReceiverEventPattern m = {0, MIDI_M_NONE, -1, 0};
  m_events.push_back(m);
  return m_events.size() - 1;
}

void GOMidiReceiverEventPatternList::DeleteEvent(unsigned index) {
  m_events.erase(m_events.begin() + index);
}
