/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSenderData.h"

GOMidiSenderData::GOMidiSenderData(GOMidiSenderType type)
  : m_type(type), m_events() {}

GOMidiSenderData::~GOMidiSenderData() {}

GOMidiSenderType GOMidiSenderData::GetType() const { return m_type; }

unsigned GOMidiSenderData::GetEventCount() const { return m_events.size(); }

GOMidiSendEvent &GOMidiSenderData::GetEvent(unsigned index) {
  return m_events[index];
}

unsigned GOMidiSenderData::AddNewEvent() {
  GOMidiSendEvent m = {0, MIDI_S_NONE, 1, 1, 0, 127};
  m_events.push_back(m);
  return m_events.size() - 1;
}

void GOMidiSenderData::DeleteEvent(unsigned index) {
  m_events.erase(m_events.begin() + index);
}
