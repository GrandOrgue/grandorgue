/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueMidiMap.h"
#include <wx/intl.h>

GOrgueMidiEvent::GOrgueMidiEvent() :
	m_miditype(MIDI_NONE),
	m_channel(-1),
	m_key(-1),
	m_value(-1),
	m_time(0)
{
}

GOrgueMidiEvent::GOrgueMidiEvent(const GOrgueMidiEvent& e) :
	m_miditype(e.m_miditype),
	m_channel(e.m_channel),
	m_key(e.m_key),
	m_value(e.m_value),
	m_device(e.m_device),
	m_time(e.m_time)
{
}

void GOrgueMidiEvent::FromMidi(const std::vector<unsigned char>& msg, GOrgueMidiMap& map)
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
		if (msg.size() == 6 && msg[0] == 0xF0 && msg[1] == 0x31 && msg[5] == 0xF7)
		{
			SetValue(((msg[2] & 0x7F) << 14) | ((msg[3] & 0x7F) << 7) | (msg[4] & 0x7F));
			SetMidiType(MIDI_SYSEX_VISCOUNT);
		}
		if (msg.size() >= 6 && msg[0] == 0xF0 && msg[1] == 0x7D && msg[2] == 0x47 && msg[3] == 0x4F && msg[msg.size() - 1] == 0xF7)
		{
			if (msg[4] == 0x00 && msg.size() == 6)
			{
				SetMidiType(MIDI_SYSEX_GO_CLEAR);
				break;
			}
			if ((msg[4] & 0xF0) == 0x10)
			{
				wxCharBuffer b(msg.size() - 7);
				char* buf = b.data();
				for(unsigned i = 0; i < msg.size() - 8; i++)
					buf[i] = msg[7 + i];
				buf[msg.size() - 8] = 0;
				SetChannel((msg[4] & 0x0F) + 1);
				SetValue(((msg[5] & 0x7F) << 7) | (msg[6] & 0x7F));
				wxString s = wxString::FromAscii(b);
				SetKey(map.GetElementByString(s));
				SetMidiType(MIDI_SYSEX_GO_SETUP);
				break;
			}
		}
		return;
	default:
		return;
	}
}

void GOrgueMidiEvent::ToMidi(std::vector<std::vector<unsigned char>>& msg, GOrgueMidiMap& map) const
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

	case MIDI_SYSEX_GO_CLEAR:
		m.resize(6);
		m[0] = 0xF0;
		m[1] = 0x7D;
		m[2] = 0x47;
		m[3] = 0x4F;
		m[4] = 0x00;
		m[5] = 0xF7;
		msg.push_back(m);
		return;

	case MIDI_SYSEX_GO_SETUP:
		{
			const wxString& s = map.GetElementByID(GetKey());
			wxCharBuffer b = s.ToAscii();
			unsigned len = s.length();
			m.resize(len + 8);
			m[0] = 0xF0;
			m[1] = 0x7D;
			m[2] = 0x47;
			m[3] = 0x4F;
			m[4] = 0x10;
			m[4] |= (GetChannel() - 1) & 0x0F;
			m[5] = (GetValue() >> 7) & 0x7F;
			m[6] = (GetValue()) & 0x7F;
			for(unsigned i = 0; i < len; i++)
				m[7 + i] = b[i] & 0x7F;
			m[7 + len] = 0xF7;
			msg.push_back(m);
		}
		return;

	case MIDI_SYSEX_HW_STRING:
		{
			const wxString& s = map.GetElementByID(GetValue());
			wxCharBuffer b = s.ToAscii();
			unsigned len = s.length();
			if (len > 16)
				len = 16;
			m.resize(21);
			m[0] = 0xF0;
			m[1] = 0x7D;
			m[2] = 0x10;
			m[3] = (GetKey()) & 0x7F;
			for(unsigned i = 0; i < len; i++)
				m[4 + i] = b[i] & 0x7F; 
			for(unsigned i = len; i < 16; i++)
				m[4 + i] = ' ';
			m[20] = 0xF7;
			msg.push_back(m);
		}
		return;

	case MIDI_SYSEX_HW_LCD:
		{
			const wxString& s = map.GetElementByID(GetValue());
			wxCharBuffer b = s.ToAscii();
			unsigned len = s.length();
			if (len > 32)
				len = 32;
			m.resize(38);
			m[0] = 0xF0;
			m[1] = 0x7D;
			m[2] = 0x01;
			m[3] = (GetKey() >> 7) & 0x7F;
			m[4] = (GetKey()) & 0x7F;
			m[5] = (GetChannel()) & 0x7F;
			for(unsigned i = 0; i < len; i++)
				m[6 + i] = b[i] & 0x7F; 
			for(unsigned i = len; i < 32; i++)
				m[6 + i] = ' ';
			m[37] = 0xF7;
			msg.push_back(m);
		}
		return;

	case MIDI_SYSEX_JOHANNUS:
	case MIDI_SYSEX_VISCOUNT:
	case MIDI_AFTERTOUCH:
	case MIDI_NONE:
	case MIDI_RESET:
		return;
	}	
}

wxString GOrgueMidiEvent::ToString(GOrgueMidiMap& map) const
{
	switch(GetMidiType())
	{
	case MIDI_NONE:
		return _("Not supported event");

	case MIDI_RESET:
		return _("reset");

	case MIDI_NOTE:
		return wxString::Format(_("note channel: %d key: %d value: %d"), GetChannel(), GetKey(), GetValue());

	case MIDI_AFTERTOUCH:
		return wxString::Format(_("aftertouch channel: %d key: %d value: %d"), GetChannel(), GetKey(), GetValue());

	case MIDI_CTRL_CHANGE:
		return wxString::Format(_("control change channel: %d key: %d value: %d"), GetChannel(), GetKey(), GetValue());

	case MIDI_PGM_CHANGE:
		return wxString::Format(_("program change channel: %d key: %d "), GetChannel(), GetKey());

	case MIDI_RPN:
		return wxString::Format(_("RPN channel: %d key: %d value: %d"), GetChannel(), GetKey(), GetValue());

	case MIDI_NRPN:
		return wxString::Format(_("NRPN change channel: %d key: %d value: %d"), GetChannel(), GetKey(), GetValue());

	case MIDI_SYSEX_JOHANNUS:
		return wxString::Format(_("sysex Johannus value: %d"), GetKey());

	case MIDI_SYSEX_VISCOUNT:
		return wxString::Format(_("sysex Viscount value: %d"), GetValue());

	case MIDI_SYSEX_GO_CLEAR:
		return _("sysex GrandOrgue clear");

	case MIDI_SYSEX_GO_SETUP:
		return wxString::Format(_("sysex GrandOrgue setup channel: %d NRPN: %d: name: %s"), GetChannel(), GetValue(), map.GetElementByID(GetKey()).c_str());

	case MIDI_SYSEX_HW_STRING:
		return wxString::Format(_("sysex Hauptwerk LCD string variable: %d - %s"), GetKey(), map.GetElementByID(GetValue()).c_str());

	case MIDI_SYSEX_HW_LCD:
		return wxString::Format(_("sysex Hauptwerk LCD display: %d color: %d - %s"), GetKey(), GetChannel(), map.GetElementByID(GetValue()).c_str());

	default:
		return wxEmptyString;
	}
}
