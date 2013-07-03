/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
	MIDI_SEND_EVENT m = { wxT(""), MIDI_S_NONE, 1, 1, 0, 127};
	m_events.push_back(m);
	return m_events.size() - 1;
}

void GOrgueMidiSenderData::DeleteEvent(unsigned index)
{
	m_events.erase(m_events.begin() + index);
}
