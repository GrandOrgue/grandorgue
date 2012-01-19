/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "GOrgueMidiEvent.h"

DEFINE_LOCAL_EVENT_TYPE( wxEVT_MIDI_ACTION )

IMPLEMENT_DYNAMIC_CLASS( GOrgueMidiEvent, wxEvent )

GOrgueMidiEvent::GOrgueMidiEvent(wxEventType type, int id) :
	wxEvent(id, type),
	m_miditype(MIDI_NONE),
	m_channel(-1),
	m_key(-1),
	m_value(-1)
{
}

GOrgueMidiEvent::GOrgueMidiEvent(const GOrgueMidiEvent& e) :
	wxEvent(e),
	m_miditype(e.m_miditype),
	m_channel(e.m_channel),
	m_key(e.m_key),
	m_value(e.m_value),
	m_device(e.m_device)
{
}

wxEvent* GOrgueMidiEvent::Clone() const
{
	return new GOrgueMidiEvent(*this);
}

void GOrgueMidiEvent::FromMidi(const std::vector<unsigned char>& msg)
{
	SetMidiType(MIDI_NONE);
	SetChannel(-1);
	SetKey(-1);
	SetValue(-1);

	if (!msg.size())
		return;
	switch(msg[0] & 0xF0)
	{
	case 0x80:
		if (msg.size() != 3)
			return;
		SetMidiType(MIDI_NOTE);
		SetChannel((msg[0] & 0x0F) + 1);
		SetKey(msg[1] & 0x7F);
		SetValue(0);
		break;
	case 0x90:
		if (msg.size() != 3)
			return;
		SetMidiType(MIDI_NOTE);
		SetChannel((msg[0] & 0x0F) + 1);
		SetKey(msg[1] & 0x7F);
		SetValue(msg[2] & 0x7F);
		break;
	case 0xB0:
		if (msg.size() != 3)
			return;
		SetMidiType(MIDI_CTRL_CHANGE);
		SetChannel((msg[0] & 0x0F) + 1);
		SetKey(msg[1] & 0x7F);
		SetValue(msg[2] & 0x7F);
		break;
	case 0xC0:
		if (msg.size() != 2)
			return;
		SetMidiType(MIDI_PGM_CHANGE);
		SetChannel((msg[0] & 0x0F) + 1);
		SetKey((msg[1] & 0x7F) + 1);
		break;
	case 0xF0:
		if (msg.size() == 1 && msg[0] == 0xFF)
		{
			SetMidiType(MIDI_RESET);
			break;
		}
		return;
	default:
		return;
	}
}

int GOrgueMidiEvent::GetEventCode()
{
	switch(GetMidiType())
	{
	case MIDI_NOTE:
		if (!GetValue())
			return -1;
		return 0x9000 | ((GetChannel() - 1) & 0x0F) << 8 | (GetKey() & 0x7F);

	case MIDI_CTRL_CHANGE:
		if (!GetValue())
			return -1;
		return 0xB000 | ((GetChannel() - 1) & 0x0F) << 8 | (GetKey() & 0x7F);
		
	case MIDI_PGM_CHANGE:
		return 0xC000 | ((GetChannel() - 1) & 0x0F) << 8 | ((GetKey() - 1) & 0x7F);

	default:
		return -1;
	}
}
