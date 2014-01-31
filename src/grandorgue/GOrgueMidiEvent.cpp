/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueMidiEvent.h"

DEFINE_LOCAL_EVENT_TYPE( wxEVT_MIDI_ACTION )

IMPLEMENT_DYNAMIC_CLASS( GOrgueMidiEvent, wxEvent )

GOrgueMidiEvent::GOrgueMidiEvent(wxEventType type, int id) :
	wxEvent(id, type),
	m_miditype(MIDI_NONE),
	m_channel(-1),
	m_key(-1),
	m_value(-1),
	m_time(0)
{
}

GOrgueMidiEvent::GOrgueMidiEvent(const GOrgueMidiEvent& e) :
	wxEvent(e),
	m_miditype(e.m_miditype),
	m_channel(e.m_channel),
	m_key(e.m_key),
	m_value(e.m_value),
	m_device(e.m_device),
	m_time(e.m_time)
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
	case 0xA0:
		if (msg.size() != 3)
			return;
		SetMidiType(MIDI_AFTERTOUCH);
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
		if (msg.size() == 9 && msg[0] == 0xF0 && msg[1] == 0x00 && msg[2] == 0x4A && msg[3] == 0x4F && msg[4] == 0x48 && msg[5] == 0x41 && msg[6] == 0x53 && msg[8] == 0xF7)
		{
			SetKey(msg[7]);
			SetMidiType(MIDI_SYSEX_JOHANNUS);
		}
		return;
	default:
		return;
	}
}

void GOrgueMidiEvent::ToMidi(std::vector<std::vector<unsigned char>>& msg)
{
	msg.resize(0);
	std::vector<unsigned char> m;
	switch(GetMidiType())
	{
	case MIDI_NOTE:
		if (GetChannel() == -1)
			return;
		m.resize(3);
		if (GetValue() == 0)
			m[0] = 0x80;
		else
			m[0] = 0x90;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = GetKey() & 0x7F;
		m[2] = GetValue() & 0x7F;
		msg.push_back(m);
		break;
	case MIDI_CTRL_CHANGE:
		if (GetChannel() == -1)
			return;
		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = GetKey() & 0x7F;
		m[2] = GetValue() & 0x7F;
		msg.push_back(m);
		return;
		
	case MIDI_PGM_CHANGE:
		if (GetChannel() == -1)
			return;
		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_BANK_SELECT_MSB & 0x7F;
		m[2] = ((GetKey() - 1  )>> 14) & 0x7F;
		msg.push_back(m);

		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_BANK_SELECT_LSB & 0x7F;
		m[2] = ((GetKey() - 1 ) >> 7) & 0x7F;
		msg.push_back(m);

		m.resize(2);
		m[0] = 0xC0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = ((GetKey() - 1 ) >> 0) & 0x7F;
		msg.push_back(m);
		return;

	case MIDI_RPN:
		if (GetChannel() == -1)
			return;
		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_RPN_MSB & 0x7F;
		m[2] = (GetKey() >> 7) & 0x7F;
		msg.push_back(m);

		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_RPN_LSB & 0x7F;
		m[2] = (GetKey() >> 0) & 0x7F;
		msg.push_back(m);

		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_DATA_ENTRY & 0x7F;
		m[2] = (GetValue() >> 0) & 0x7F;
		msg.push_back(m);

		return;

	case MIDI_NRPN:
		if (GetChannel() == -1)
			return;
		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_NRPN_MSB & 0x7F;
		m[2] = (GetKey() >> 7) & 0x7F;
		msg.push_back(m);

		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_NRPN_LSB & 0x7F;
		m[2] = (GetKey() >> 0) & 0x7F;
		msg.push_back(m);

		m.resize(3);
		m[0] = 0xB0;
		m[0] |= (GetChannel() - 1) & 0x0F;
		m[1] = MIDI_CTRL_DATA_ENTRY & 0x7F;
		m[2] = (GetValue() >> 0) & 0x7F;
		msg.push_back(m);

		return;

	case MIDI_SYSEX_JOHANNUS:
	case MIDI_AFTERTOUCH:
	case MIDI_NONE:
	case MIDI_RESET:
		return;
	}	
}
