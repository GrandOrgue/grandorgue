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
 */

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueMidiReceiver.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueManual.h"
#include "GOrgueEnclosure.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

GOrgueMidiReceiver::GOrgueMidiReceiver(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE type):
	m_organfile(organfile),
	m_type(type),
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

void GOrgueMidiReceiver::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_events.resize(0);

	int event_cnt = cfg.ReadInteger(CMBSetting, group, wxT("NumberOfMIDIEvents"), -1, 255, false);
	if (event_cnt >= 0)
	{
		wxString buffer;

		m_events.resize(event_cnt);
		for(unsigned i = 0; i < m_events.size(); i++)
		{
			buffer.Printf(wxT("MIDIDevice%03d"), i + 1);
			m_events[i].device = cfg.ReadString(CMBSetting, group, buffer, 100, false);
			buffer.Printf(wxT("MIDIChannel%03d"), i + 1);
			m_events[i].channel = cfg.ReadInteger(CMBSetting, group, buffer, -1, 16);
			if (m_type == MIDI_RECV_MANUAL)
			{
				buffer.Printf(wxT("MIDIKeyShift%03d"), i + 1);
				m_events[i].key = cfg.ReadInteger(CMBSetting, group, buffer, -35, 35);
				m_events[i].type = MIDI_M_NOTE;
				m_events[i].low_key = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDILowerKey%03d"), i + 1), 0, 127, false, 0);
				m_events[i].high_key = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIUpperKey%03d"), i + 1), 0, 127, false, 127);
				m_events[i].low_velocity = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), 0, 127, false, 1);
				m_events[i].high_velocity = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), 0, 127, false, 127);
				continue;
			}
			buffer.Printf(wxT("MIDIKey%03d"), i + 1);
			m_events[i].key = cfg.ReadInteger(CMBSetting, group, buffer, 0, 0x200000);
			
			if (m_type == MIDI_RECV_ENCLOSURE)
			{
				m_events[i].type = MIDI_M_CTRL_CHANGE;
				m_events[i].low_velocity = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), 0, 127, false, 0);
				m_events[i].high_velocity = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), 0, 127, false, 127);
				continue;
			}
			
			buffer.Printf(wxT("MIDIEventType%03d"), i + 1);
			m_events[i].type = (midi_match_message_type)cfg.ReadEnum(CMBSetting, group, buffer, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
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
			if (m_Index == -1 || m_Index >= (int) m_organfile->GetSettings().GetSetterCount())
			{
				m_events.resize(0);
				return;
			}
			int what = m_organfile->GetSettings().GetSetterEvent(m_Index);
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
			int what = m_organfile->GetSettings().GetStopChangeEvent();
			m_events[0].channel = ((what >> 8) & 0xF) + 1;
			m_events[0].key = cfg.ReadInteger(UserSetting, group, wxT("StopControlMIDIKeyNumber"), -1, 127, false);
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
			if (m_Index != -1)
			{
				unsigned midi_index = m_organfile->GetManual(m_Index)->GetMIDIInputNumber();
				if (!midi_index || midi_index > m_organfile->GetSettings().GetManualCount())
				{
					m_events.resize(0);
					return;
				}
				int what = m_organfile->GetSettings().GetManualEvent(midi_index - 1);
				m_events[0].channel = ((what >> 8) & 0xF) + 1;
				if ((what & 0xF000) == 0)
				{
					m_events.resize(0);
					return;
				}
			}
			m_events[0].key = cfg.ReadInteger(UserSetting, group, wxT("MIDIProgramChangeNumber"), 0, 128, false);
			if (!m_events[0].key)
			{
				m_events.resize(0);
				return;
			}
		}
		if (m_type == MIDI_RECV_MANUAL)
		{
			if (m_Index == -1)
			{
				m_events.resize(0);
				return;
			}
			unsigned midi_index = m_organfile->GetManual(m_Index)->GetMIDIInputNumber();
			if (!midi_index || midi_index > m_organfile->GetSettings().GetManualCount())
			{
				m_events.resize(0);
				return;
			}
			int what = m_organfile->GetSettings().GetManualEvent(midi_index - 1);
			if (!m_organfile->GetManual(m_Index)->IsDisplayed() || (what & 0xF000) == 0)
			{
				m_events.resize(0);
				return;
			}
			m_events[0].type = MIDI_M_NOTE;
			m_events[0].channel = ((what >> 8) & 0xF) + 1;
			m_events[0].key = (what & 0xFF) < 0x7F ? (what & 0xFF) : ((what & 0xFF) - 140);
			m_events[0].low_key = 0;
			m_events[0].high_key = 127;
			m_events[0].low_velocity = 1;
			m_events[0].high_velocity = 127;
		}
		if (m_type == MIDI_RECV_ENCLOSURE)
		{
			if (m_Index  == -1)
			{
				m_events.resize(0);
				return;
			}
			unsigned midi_index = m_organfile->GetEnclosure(m_Index)->GetMIDIInputNumber();
			if (!midi_index || midi_index > m_organfile->GetSettings().GetEnclosureCount())
			{
				m_events.resize(0);
				return;
			}
			int what = m_organfile->GetSettings().GetEnclosureEvent(midi_index - 1);
			if ((what & 0xF000) == 0)
			{
				m_events.resize(0);
				return;
			}
			m_events[0].type = MIDI_M_CTRL_CHANGE;
			m_events[0].channel = ((what >> 8) & 0xF) + 1;
			m_events[0].key = what & 0x7F;
			m_events[0].low_velocity = 0;
			m_events[0].high_velocity = 127;
		}
	}
}

void GOrgueMidiReceiver::Save(GOrgueConfigWriter& cfg, wxString group)
{
	wxString buffer;

	cfg.Write(group, wxT("NumberOfMIDIEvents"), (int)m_events.size());
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		buffer.Printf(wxT("MIDIDevice%03d"), i + 1);
		cfg.Write(group, buffer, m_events[i].device);
		buffer.Printf(wxT("MIDIChannel%03d"), i + 1);
		cfg.Write(group, buffer, m_events[i].channel);
		if (m_type == MIDI_RECV_MANUAL)
		{
			buffer.Printf(wxT("MIDIKeyShift%03d"), i + 1);
			cfg.Write(group, buffer, m_events[i].key);
			cfg.Write(group, wxString::Format(wxT("MIDILowerKey%03d"), i + 1), m_events[i].low_key);
			cfg.Write(group, wxString::Format(wxT("MIDIUpperKey%03d"), i + 1), m_events[i].high_key);
			cfg.Write(group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), m_events[i].low_velocity);
			cfg.Write(group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), m_events[i].high_velocity);
			continue;
		}
		buffer.Printf(wxT("MIDIKey%03d"), i + 1);
		cfg.Write(group, buffer, m_events[i].key);

		if (m_type == MIDI_RECV_ENCLOSURE)
		{
			cfg.Write(group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), m_events[i].low_velocity);
			cfg.Write(group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), m_events[i].high_velocity);
			continue;
		}

		buffer.Printf(wxT("MIDIEventType%03d"), i + 1);
		cfg.Write(group, buffer, m_events[i].type, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
	}
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e)
{
	int tmp;
	return Match(e, tmp);
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e, int& value)
{
	int tmp;
	return Match(e, tmp, value);
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e, int& key, int& value)
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
			if (e.GetMidiType() == MIDI_NOTE || e.GetMidiType() == MIDI_AFTERTOUCH)
			{
				if (e.GetKey() < m_events[i].low_key || e.GetKey() > m_events[i].high_key)
					continue;
				key = e.GetKey() + m_organfile->GetSettings().GetTranspose() + m_events[i].key;
				value = e.GetValue() - m_events[i].low_velocity;
				value *= 127 / (m_events[i].high_velocity - m_events[i].low_velocity + 0.00000001);
				if (value < 0)
					value = 0;
				if (value > 127)
					value = 127;
				if (e.GetValue() < m_events[i].low_velocity)
					return MIDI_MATCH_OFF;
				if (e.GetValue() <= m_events[i].high_velocity)
					return MIDI_MATCH_ON;
				continue;
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
				value = value - m_events[i].low_velocity;
				value *= 127 / (m_events[i].high_velocity - m_events[i].low_velocity + 0.00000001);
				if (value < 0)
					value = 0;
				if (value > 127)
					value = 127;
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

GrandOrgueFile* GOrgueMidiReceiver::GetOrganfile()
{
	return m_organfile;
}
