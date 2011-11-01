/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueMidi.h"
#include "GOrgueMidiReceiver.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueManual.h"
#include "GOrgueEnclosure.h"
#include "GOrgueSound.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

extern GOrgueSound* g_sound;

GOrgueMidiReceiver::GOrgueMidiReceiver(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE type):
	m_organfile(organfile),
	m_type(type),
	m_Key(-1),
	m_Index(-1),
	m_events(0)
{
}

MIDI_RECEIVER_TYPE GOrgueMidiReceiver::GetType() const
{
	return m_type;
}

void GOrgueMidiReceiver::SetIndex(int index)
{
	m_Index = index;
}

const struct IniFileEnumEntry GOrgueMidiReceiver::m_MidiTypes[] = {
	{ wxT("ControlChange"), MIDI_M_CTRL_CHANGE },
	{ wxT("Note"), MIDI_M_NOTE },
	{ wxT("ProgramChange"), MIDI_M_PGM_CHANGE },
	{ wxT("ControlChangeOn"), MIDI_M_CTRL_CHANGE_ON },
	{ wxT("ControlChangeOff"), MIDI_M_CTRL_CHANGE_OFF },
	{ wxT("NoteOn"), MIDI_M_NOTE_ON },
	{ wxT("NoteOff"), MIDI_M_NOTE_OFF },
};

void GOrgueMidiReceiver::Load(IniFileConfig& cfg, wxString group)
{
	m_events.resize(0);

	int event_cnt = cfg.ReadInteger(group, wxT("NumberOfMIDIEvents"), -1, 255, false);
	if (event_cnt >= 0)
	{
		wxString buffer;

		m_events.resize(event_cnt);
		for(unsigned i = 0; i < m_events.size(); i++)
		{
			buffer.Printf(wxT("MIDIDevice%03d"), i + 1);
			m_events[i].device = cfg.ReadString(group, buffer, 100, false);
			buffer.Printf(wxT("MIDIChannel%03d"), i + 1);
			m_events[i].channel = cfg.ReadInteger(group, buffer, -1, 16);
			if (m_type == MIDI_RECV_MANUAL)
			{
				buffer.Printf(wxT("MIDIKeyShift%03d"), i + 1);
				m_events[i].key = cfg.ReadInteger(group, buffer, -35, 35);
				continue;
			}
			buffer.Printf(wxT("MIDIKey%03d"), i + 1);
			m_events[i].key = cfg.ReadInteger(group, buffer, 0, 0x200000);
			
			if (m_type == MIDI_RECV_ENCLOSURE)
				continue;
			
			buffer.Printf(wxT("MIDIEventType%03d"), i + 1);
			m_events[i].type = (midi_match_message_type)cfg.ReadEnum(group, buffer, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
		}
	}
	else
	{
		m_events.resize(1);
		m_events[0].channel = -1;
		m_events[0].device = wxT("");
		m_events[0].type = MIDI_M_NONE;
		m_events[0].key = -1;
		if (m_type == MIDI_RECV_SETTER)
		{
			if (m_Index == -1)
			{
				m_events.resize(0);
				return;
			}
			int what = g_sound->GetMidi().GetMidiEventByChannel(m_Index);
			m_events[0].channel = ((what >> 8) & 0xF) + 1;
			m_events[0].key = (what & 0x7F);
			if ((what & 0xF000) == 0xC000)
			{
				m_events[0].type = MIDI_M_PGM_CHANGE;
				m_events[0].key++;
			}
			if ((what & 0xF000) == 0xB000)
			{
				m_events[0].type = MIDI_M_CTRL_CHANGE;
			}
			if ((what & 0xF000) == 0x9000)
			{
				m_events[0].type = MIDI_M_NOTE;
			}
			if ((what & 0xF000) == 0)
			{
				m_events.resize(0);
			}
			
		}
		if (m_type == MIDI_RECV_DRAWSTOP)
		{
			int what = g_sound->GetMidi().GetStopMidiEvent();
			m_events[0].channel = ((what >> 8) & 0xF) + 1;
			m_events[0].key = cfg.ReadInteger(group, wxT("StopControlMIDIKeyNumber"), -1, 127, false);
			if (m_events[0].key == -1)
			{
				m_events.resize(0);
				return;
			}
			if ((what & 0xF000) == 0xC000)
			{
				m_events[0].type = MIDI_M_PGM_CHANGE;
				m_events[0].key++;
			}
			if ((what & 0xF000) == 0xB000)
			{
				m_events[0].type = MIDI_M_CTRL_CHANGE;
			}
			if ((what & 0xF000) == 0x9000)
			{
				m_events[0].type = MIDI_M_NOTE;
			}
			if ((what & 0xF000) == 0)
			{
				m_events.resize(0);
			}
		}
		if (m_type == MIDI_RECV_BUTTON)
		{
			m_events[0].type = MIDI_M_PGM_CHANGE;
			if (m_Index > 6)
			{
				m_events.resize(0);
				return;
			}
			if (m_Index != -1)
			{
				int what = g_sound->GetMidi().GetManualMidiEvent(m_organfile->GetManual(m_Index)->GetMIDIInputNumber());
				m_events[0].channel = ((what >> 8) & 0xF) + 1;
			}
			m_events[0].key = cfg.ReadInteger(group, wxT("MIDIProgramChangeNumber"), 0, 128, false);
			if (!m_events[0].key)
			{
				m_events.resize(0);
				return;
			}
		}
		if (m_type == MIDI_RECV_MANUAL)
		{
			if (m_Index > 6 || m_Index == -1)
			{
				m_events.resize(0);
				return;
			}
			int what = g_sound->GetMidi().GetManualMidiEvent(m_organfile->GetManual(m_Index)->GetMIDIInputNumber());
			if (!m_organfile->GetManual(m_Index)->IsDisplayed() || (what & 0xF000) == 0)
			{
				m_events.resize(0);
				return;
			}
			m_events[0].type = MIDI_M_NOTE;
			m_events[0].channel = ((what >> 8) & 0xF) + 1;
			m_events[0].key = (what & 0xFF) < 0x7F ? (what & 0xFF) : ((what & 0xFF) - 140);
		}
		if (m_type == MIDI_RECV_ENCLOSURE)
		{
			if (m_Index >= 6 || m_Index  == -1)
			{
				m_events.resize(0);
				return;
			}
			int what = g_sound->GetMidi().GetMidiEventByChannel(m_organfile->GetEnclosure(m_Index)->GetMIDIInputNumber() + 1);
			if ((what & 0xF000) == 0)
			{
				m_events.resize(0);
				return;
			}
			m_events[0].type = MIDI_M_CTRL_CHANGE;
			m_events[0].channel = ((what >> 8) & 0xF) + 1;
			m_events[0].key = what & 0x7F;
		}
	}
}

void GOrgueMidiReceiver::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
	wxString buffer;

	cfg.SaveHelper(prefix, group, wxT("NumberOfMIDIEvents"), m_events.size());
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		buffer.Printf(wxT("MIDIDevice%03d"), i + 1);
		cfg.SaveHelper(prefix, group, buffer, m_events[i].device);
		buffer.Printf(wxT("MIDIChannel%03d"), i + 1);
		cfg.SaveHelper(prefix, group, buffer, m_events[i].channel);
		if (m_type == MIDI_RECV_MANUAL)
		{
			buffer.Printf(wxT("MIDIKeyShift%03d"), i + 1);
			cfg.SaveHelper(prefix, group, buffer, m_events[i].key);
			continue;
		}
		buffer.Printf(wxT("MIDIKey%03d"), i + 1);
		cfg.SaveHelper(prefix, group, buffer, m_events[i].key);

		if (m_type == MIDI_RECV_ENCLOSURE)
			continue;

		buffer.Printf(wxT("MIDIEventType%03d"), i + 1);
		cfg.SaveHelper(prefix, group, buffer, m_events[i].type, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
	}
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e)
{
	int tmp;
	return Match(e, tmp);
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e, int& value)
{
	value = 0;
	for(unsigned i = 0; i < m_events.size();i++)
	{
		if (m_events[i].channel != -1 && m_events[i].channel != e.GetChannel())
			continue;
		if (!m_events[i].device.IsEmpty() && m_events[i].device != e.GetDevice())
			continue;
		if (m_type == MIDI_RECV_MANUAL)
		{
			if (e.GetMidiType() == MIDI_NOTE)
			{
				value = e.GetKey() + g_sound->GetMidi().GetTranspose() + m_events[i].key;
				if (e.GetValue())
					return MIDI_MATCH_ON;
				else
					return MIDI_MATCH_OFF;
			}
			if (e.GetMidiType() == MIDI_CTRL_CHANGE && 
			    e.GetKey() == MIDI_CTRL_NOTES_OFF)
				return MIDI_MATCH_RESET;

			if (e.GetMidiType() == MIDI_CTRL_CHANGE && 
			    e.GetKey() == MIDI_CTRL_SOUNDS_OFF)
				return MIDI_MATCH_RESET;

			continue;
		}
		if (m_type == MIDI_RECV_ENCLOSURE)
		{
			if (e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].key == e.GetKey())
			{
				value = e.GetValue();
				return MIDI_MATCH_CHANGE;
			}
			continue;
		}
		if (e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE && m_events[i].key == e.GetKey())
		{
			if (e.GetValue())
				return MIDI_MATCH_ON;
			else
				return MIDI_MATCH_OFF;
		}
		if (e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_ON && m_events[i].key == e.GetKey() && e.GetValue())
			return MIDI_MATCH_CHANGE;
		if (e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_OFF && m_events[i].key == e.GetKey() && !e.GetValue())
			return MIDI_MATCH_CHANGE;

		if (e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].type == MIDI_M_CTRL_CHANGE && m_events[i].key == e.GetKey())
		{
			if (e.GetValue())
				return MIDI_MATCH_ON;
			else
				return MIDI_MATCH_OFF;
		}
		if (e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].type == MIDI_M_CTRL_CHANGE_ON && m_events[i].key == e.GetKey() && e.GetValue())
			return MIDI_MATCH_CHANGE;
		if (e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].type == MIDI_M_CTRL_CHANGE_OFF && m_events[i].key == e.GetKey() && !e.GetValue())
			return MIDI_MATCH_CHANGE;

		if (e.GetMidiType() == MIDI_PGM_CHANGE && m_events[i].type == MIDI_M_PGM_CHANGE && m_events[i].key == e.GetKey())
		{
			return MIDI_MATCH_CHANGE;
		}
	}
	return MIDI_MATCH_NONE;
}

unsigned GOrgueMidiReceiver::GetEventCount() const
{
	return m_events.size();
}

MIDI_MATCH_EVENT& GOrgueMidiReceiver::GetEvent(unsigned index)
{
	return m_events[index];
}

unsigned GOrgueMidiReceiver::AddNewEvent()
{
	MIDI_MATCH_EVENT m = { wxT(""), MIDI_M_NONE, -1, 0 };
	m_events.push_back(m);
	return m_events.size() - 1;
}

void GOrgueMidiReceiver::DeleteEvent(unsigned index)
{
	m_events.erase(m_events.begin() + index);
}

GOrgueSettings& GOrgueMidiReceiver::GetSettings()
{
	return m_organfile->GetSettings();
}
