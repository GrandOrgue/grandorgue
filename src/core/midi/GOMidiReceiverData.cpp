/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiReceiverData.h"

GOMidiReceiverData::GOMidiReceiverData(GOMidiReceiverType type)
  : m_type(type), m_events(0) {}

GOMidiReceiverData::~GOMidiReceiverData() {}

GOMidiReceiverType GOMidiReceiverData::GetType() const { return m_type; }

unsigned GOMidiReceiverData::GetEventCount() const { return m_events.size(); }

GOMidiReceiveEvent &GOMidiReceiverData::GetEvent(unsigned index) {
  return m_events[index];
}

unsigned GOMidiReceiverData::AddNewEvent() {
  GOMidiReceiveEvent m = {0, MIDI_M_NONE, -1, 0};
  m_events.push_back(m);
  return m_events.size() - 1;
}

void GOMidiReceiverData::DeleteEvent(unsigned index) {
  m_events.erase(m_events.begin() + index);
}
