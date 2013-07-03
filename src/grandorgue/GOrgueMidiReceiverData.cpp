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

#include "GOrgueMidiReceiverData.h"

GOrgueMidiReceiverData::GOrgueMidiReceiverData(MIDI_RECEIVER_TYPE type):
	m_type(type),
	m_events(0)
{
}

GOrgueMidiReceiverData::~GOrgueMidiReceiverData()
{
}

MIDI_RECEIVER_TYPE GOrgueMidiReceiverData::GetType() const
{
	return m_type;
}

unsigned GOrgueMidiReceiverData::GetEventCount() const
{
	return m_events.size();
}

MIDI_MATCH_EVENT& GOrgueMidiReceiverData::GetEvent(unsigned index)
{
	return m_events[index];
}

unsigned GOrgueMidiReceiverData::AddNewEvent()
{
	MIDI_MATCH_EVENT m = { wxT(""), MIDI_M_NONE, -1, 0 };
	m_events.push_back(m);
	return m_events.size() - 1;
}

void GOrgueMidiReceiverData::DeleteEvent(unsigned index)
{
	m_events.erase(m_events.begin() + index);
}
