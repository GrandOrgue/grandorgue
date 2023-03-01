/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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
  m_events.emplace_back();
  return m_events.size() - 1;
}

void GOMidiSenderEventPatternList::DeleteEvent(unsigned index) {
  m_events.erase(m_events.begin() + index);
}
