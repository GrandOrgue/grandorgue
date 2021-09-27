/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueMidiSenderData.h"

GOrgueMidiSenderData::GOrgueMidiSenderData(MIDI_SENDER_TYPE type) :
	m_type(type),
	m_events()
{
}

GOrgueMidiSenderData::~GOrgueMidiSenderData()
{
}

MIDI_SENDER_TYPE GOrgueMidiSenderData::GetType() const
{
	return m_type;
}

unsigned GOrgueMidiSenderData::GetEventCount() const
{
	return m_events.size();
}

MIDI_SEND_EVENT& GOrgueMidiSenderData::GetEvent(unsigned index)
{
	return m_events[index];
}

unsigned GOrgueMidiSenderData::AddNewEvent()
{
	MIDI_SEND_EVENT m = { 0, MIDI_S_NONE, 1, 1, 0, 127};
	m_events.push_back(m);
	return m_events.size() - 1;
}

void GOrgueMidiSenderData::DeleteEvent(unsigned index)
{
	m_events.erase(m_events.begin() + index);
}
