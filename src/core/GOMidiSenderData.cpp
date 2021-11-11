/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidiSenderData.h"

GOMidiSenderData::GOMidiSenderData(MIDI_SENDER_TYPE type) :
	m_type(type),
	m_events()
{
}

GOMidiSenderData::~GOMidiSenderData()
{
}

MIDI_SENDER_TYPE GOMidiSenderData::GetType() const
{
	return m_type;
}

unsigned GOMidiSenderData::GetEventCount() const
{
	return m_events.size();
}

MIDI_SEND_EVENT& GOMidiSenderData::GetEvent(unsigned index)
{
	return m_events[index];
}

unsigned GOMidiSenderData::AddNewEvent()
{
	MIDI_SEND_EVENT m = { 0, MIDI_S_NONE, 1, 1, 0, 127};
	m_events.push_back(m);
	return m_events.size() - 1;
}

void GOMidiSenderData::DeleteEvent(unsigned index)
{
	m_events.erase(m_events.begin() + index);
}
