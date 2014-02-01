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

#include "GOrgueMidiReceiver.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueEnclosure.h"
#include "GOrgueManual.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueMidiMap.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

GOrgueMidiReceiver::GOrgueMidiReceiver(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE type):
	GOrgueMidiReceiverData(type),
	m_organfile(organfile),
	m_Index(-1),
	m_last()
{
}

void GOrgueMidiReceiver::SetIndex(int index)
{
	m_Index = index;
}

const struct IniFileEnumEntry GOrgueMidiReceiver::m_MidiTypes[] = {
	{ wxT("ControlChange"), MIDI_M_CTRL_CHANGE },
	{ wxT("Note"), MIDI_M_NOTE },
	{ wxT("ProgramChange"), MIDI_M_PGM_CHANGE },
	{ wxT("SysExJohannus"), MIDI_M_SYSEX_JOHANNUS },
	{ wxT("RPN"), MIDI_M_RPN },
	{ wxT("NRPN"), MIDI_M_NRPN },
	{ wxT("RPNOn"), MIDI_M_RPN_ON },
	{ wxT("RPNOff"), MIDI_M_RPN_OFF },
	{ wxT("NRPNOn"), MIDI_M_NRPN_ON },
	{ wxT("NRPNOff"), MIDI_M_NRPN_OFF },
	{ wxT("ControlChangeOn"), MIDI_M_CTRL_CHANGE_ON },
	{ wxT("ControlChangeOff"), MIDI_M_CTRL_CHANGE_OFF },
	{ wxT("NoteOn"), MIDI_M_NOTE_ON },
	{ wxT("NoteOff"), MIDI_M_NOTE_OFF },
	{ wxT("NoteNoVelocity"), MIDI_M_NOTE_NO_VELOCITY },
	{ wxT("NoteShortOctave"), MIDI_M_NOTE_SHORT_OCTAVE },
};

void GOrgueMidiReceiver::Load(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map)
{
	m_events.resize(0);

	if (m_organfile && !m_organfile->GetSettings().GetODFCheck())
	{
		/* Skip old style entries */
		if (m_type == MIDI_RECV_DRAWSTOP)
			cfg.ReadInteger(ODFSetting, group, wxT("StopControlMIDIKeyNumber"), -1, 127, false);
		if (m_type == MIDI_RECV_BUTTON)
			cfg.ReadInteger(ODFSetting, group, wxT("MIDIProgramChangeNumber"), 0, 128, false);
	}

	int event_cnt = cfg.ReadInteger(CMBSetting, group, wxT("NumberOfMIDIEvents"), -1, 255, false);
	if (event_cnt >= 0)
	{

		m_events.resize(event_cnt);
		for(unsigned i = 0; i < m_events.size(); i++)
		{
			m_events[i].device = map.GetDeviceByString(cfg.ReadString(CMBSetting, group, wxString::Format(wxT("MIDIDevice%03d"), i + 1), false));
			m_events[i].channel = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIChannel%03d"), i + 1), -1, 16);
			midi_match_message_type default_type = MIDI_M_PGM_CHANGE;
			if (m_type == MIDI_RECV_MANUAL)
				default_type = MIDI_M_NOTE;
			if (m_type == MIDI_RECV_ENCLOSURE)
				default_type = MIDI_M_CTRL_CHANGE;
			m_events[i].type = (midi_match_message_type)cfg.ReadEnum(CMBSetting, group, wxString::Format(wxT("MIDIEventType%03d"), i + 1), 
										 m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]), false, default_type);
			if (HasDebounce(m_events[i].type))
				m_events[i].debounce_time = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIDebounce%03d"), i + 1), 0, 3000, false, 0);

			if (m_type == MIDI_RECV_MANUAL)
			{
				m_events[i].key = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIKeyShift%03d"), i + 1), -35, 35);
				m_events[i].type = (midi_match_message_type)cfg.ReadEnum(CMBSetting, group, wxString::Format(wxT("MIDIEventType%03d"), i + 1), 
											 m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]), false, MIDI_M_NOTE);
				m_events[i].low_key = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDILowerKey%03d"), i + 1), 0, 127, false, 0);
				m_events[i].high_key = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIUpperKey%03d"), i + 1), 0, 127, false, 127);
				m_events[i].low_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), 0, 127, false, 1);
				m_events[i].high_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), 0, 127, false, 127);
				continue;
			}
			m_events[i].key = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIKey%03d"), i + 1), 0, 0x200000);
			
			if (m_type == MIDI_RECV_ENCLOSURE)
			{
				m_events[i].low_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), 0, 127, false, 0);
				m_events[i].high_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), 0, 127, false, 127);
				continue;
			}
			if (HasLowerLimit(m_events[i].type))
				m_events[i].low_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDILowerLimit%03d"), i + 1), 0, 127, false, 0);
			if (HasUpperLimit(m_events[i].type))
				m_events[i].high_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDIUpperLimit%03d"), i + 1), 0, 127, false, 1);
			
		}
	}
	else
	{
		m_events.resize(0);
		if (!m_organfile)
			return;
		unsigned index = 0;

		if (m_type == MIDI_RECV_SETTER)
		{
			index = m_Index;
		}
		if (m_type == MIDI_RECV_MANUAL)
		{
			if (m_Index == -1)
				return;

			index = m_organfile->GetManual(m_Index)->GetMIDIInputNumber();
		}
		if (m_type == MIDI_RECV_ENCLOSURE)
		{
			if (m_Index  == -1)
				return;

			index = m_organfile->GetEnclosure(m_Index)->GetMIDIInputNumber();
		}
		GOrgueMidiReceiver* recv = m_organfile->GetSettings().FindMidiEvent(m_type, index);
		if (!recv)
			return;
		
		m_events = recv->m_events;
	}
}

void GOrgueMidiReceiver::Save(GOrgueConfigWriter& cfg, wxString group, GOrgueMidiMap& map)
{
	cfg.WriteInteger(group, wxT("NumberOfMIDIEvents"), m_events.size());
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		cfg.WriteString(group, wxString::Format(wxT("MIDIDevice%03d"), i + 1), map.GetDeviceByID(m_events[i].device));
		cfg.WriteInteger(group, wxString::Format(wxT("MIDIChannel%03d"), i + 1), m_events[i].channel);
		cfg.WriteEnum(group, wxString::Format(wxT("MIDIEventType%03d"), i + 1), m_events[i].type, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
		if (HasDebounce(m_events[i].type))
			cfg.WriteInteger(group, wxString::Format(wxT("MIDIDebounce%03d"), i + 1), m_events[i].debounce_time);

		if (m_type == MIDI_RECV_MANUAL)
		{
			cfg.WriteInteger(group, wxString::Format(wxT("MIDIKeyShift%03d"), i + 1), m_events[i].key);
			cfg.WriteInteger(group, wxString::Format(wxT("MIDILowerKey%03d"), i + 1), m_events[i].low_key);
			cfg.WriteInteger(group, wxString::Format(wxT("MIDIUpperKey%03d"), i + 1), m_events[i].high_key);
			cfg.WriteInteger(group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), m_events[i].low_value);
			cfg.WriteInteger(group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), m_events[i].high_value);
			continue;
		}
		cfg.WriteInteger(group, wxString::Format(wxT("MIDIKey%03d"), i + 1), m_events[i].key);

		if (m_type == MIDI_RECV_ENCLOSURE)
		{
			cfg.WriteInteger(group, wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1), m_events[i].low_value);
			cfg.WriteInteger(group, wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1), m_events[i].high_value);
			continue;
		}
		if (HasLowerLimit(m_events[i].type))
			cfg.WriteInteger(group, wxString::Format(wxT("MIDILowerLimit%03d"), i + 1), m_events[i].low_value);
		if (HasUpperLimit(m_events[i].type))
			cfg.WriteInteger(group, wxString::Format(wxT("MIDIUpperLimit%03d"), i + 1), m_events[i].high_value);
	}
}

bool GOrgueMidiReceiver::HasDebounce(midi_match_message_type type)
{
	if (m_type == MIDI_RECV_MANUAL)
		return false;
	if (m_type == MIDI_RECV_ENCLOSURE)
		return false;
	if (type == MIDI_M_PGM_CHANGE ||
	    type == MIDI_M_NOTE_OFF ||
	    type == MIDI_M_CTRL_CHANGE_OFF ||
	    type == MIDI_M_RPN_OFF ||
	    type == MIDI_M_NRPN_OFF ||
	    type == MIDI_M_NOTE_ON ||
	    type == MIDI_M_CTRL_CHANGE_ON ||
	    type == MIDI_M_RPN_ON ||
	    type == MIDI_M_NRPN_ON ||
	    type == MIDI_M_SYSEX_JOHANNUS)
		return true;
	return false;
}

bool GOrgueMidiReceiver::HasLowerLimit(midi_match_message_type type)
{
	if (m_type == MIDI_RECV_MANUAL)
		return false;
	if (m_type == MIDI_RECV_ENCLOSURE)
		return false;
	if (type == MIDI_M_NOTE ||
	    type == MIDI_M_CTRL_CHANGE ||
	    type == MIDI_M_RPN ||
	    type == MIDI_M_NRPN ||
	    type == MIDI_M_NOTE_OFF ||
	    type == MIDI_M_CTRL_CHANGE_OFF ||
	    type == MIDI_M_RPN_OFF ||
	    type == MIDI_M_NRPN_OFF)
		return true;
	return false;
}

bool GOrgueMidiReceiver::HasUpperLimit(midi_match_message_type type)
{
	if (m_type == MIDI_RECV_MANUAL)
		return false;
	if (m_type == MIDI_RECV_ENCLOSURE)
		return false;
	if (type == MIDI_M_NOTE ||
	    type == MIDI_M_CTRL_CHANGE ||
	    type == MIDI_M_RPN ||
	    type == MIDI_M_NRPN ||
	    type == MIDI_M_NOTE_ON ||
	    type == MIDI_M_CTRL_CHANGE_ON ||
	    type == MIDI_M_RPN_ON ||
	    type == MIDI_M_NRPN_ON)
		return true;
	return false;
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::debounce(const GOrgueMidiEvent& e, MIDI_MATCH_TYPE event, unsigned index)
{
	if (m_events.size() != m_last.size())
		m_last.resize(m_events.size());
	if (e.GetTime() < m_last[index] + m_events[index].debounce_time)
		return MIDI_MATCH_NONE;
	m_last[index] = e.GetTime();
	return event;
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e)
{
	int tmp;
	return Match(e, tmp);
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e, int& value)
{
	int tmp;
	return Match(e, NULL, tmp, value);
}

MIDI_MATCH_TYPE GOrgueMidiReceiver::Match(const GOrgueMidiEvent& e, const unsigned midi_map[128], int& key, int& value)
{
	value = 0;
	for(unsigned i = 0; i < m_events.size();i++)
	{
		if (m_events[i].channel != -1 && m_events[i].channel != e.GetChannel())
			continue;
		if (m_events[i].device != 0 && m_events[i].device != e.GetDevice())
			continue;
		if (m_type == MIDI_RECV_MANUAL)
		{
			if (m_events[i].type != MIDI_M_NOTE && m_events[i].type != MIDI_M_NOTE_NO_VELOCITY &&
			    m_events[i].type != MIDI_M_NOTE_SHORT_OCTAVE)
				continue;
			if (e.GetMidiType() == MIDI_NOTE || e.GetMidiType() == MIDI_AFTERTOUCH)
			{
				if (e.GetKey() < m_events[i].low_key || e.GetKey() > m_events[i].high_key)
					continue;
				key = e.GetKey();
				if (m_events[i].type == MIDI_M_NOTE_SHORT_OCTAVE)
				{
					int no = e.GetKey() - m_events[i].low_key;
					if (no <= 3)
						continue;
					if (no == 4 || no == 6 || no == 8)
						key -= 4;
				}
				key = key + m_organfile->GetSettings().GetTranspose() + m_events[i].key;
				if (key < 0)
					continue;
				if (key > 127)
					continue;
				if (midi_map && m_events[i].type != MIDI_M_NOTE_SHORT_OCTAVE)
					key = midi_map[key];
				if (m_events[i].type == MIDI_M_NOTE_NO_VELOCITY)
				{
					value = e.GetValue() ? 127 : 0;
				}
				else
				{
					value = e.GetValue() - m_events[i].low_value;
					value *= 127 / (m_events[i].high_value - m_events[i].low_value + 0.00000001);
				}
				if (value < 0)
					value = 0;
				if (value > 127)
					value = 127;
				if (e.GetValue() < m_events[i].low_value)
					return MIDI_MATCH_OFF;
				if (e.GetValue() <= m_events[i].high_value)
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
			if (m_events[i].type == MIDI_M_CTRL_CHANGE && e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].key == e.GetKey())
			{
				value = e.GetValue();
				value = value - m_events[i].low_value;
				value *= 127 / (m_events[i].high_value - m_events[i].low_value + 0.00000001);
				if (value < 0)
					value = 0;
				if (value > 127)
					value = 127;
				return MIDI_MATCH_CHANGE;
			}
			if (m_events[i].type == MIDI_M_RPN && e.GetMidiType() == MIDI_RPN && m_events[i].key == e.GetKey())
			{
				value = e.GetValue();
				value = value - m_events[i].low_value;
				value *= 127 / (m_events[i].high_value - m_events[i].low_value + 0.00000001);
				if (value < 0)
					value = 0;
				if (value > 127)
					value = 127;
				return MIDI_MATCH_CHANGE;
			}
			if (m_events[i].type == MIDI_M_NRPN && e.GetMidiType() == MIDI_NRPN && m_events[i].key == e.GetKey())
			{
				value = e.GetValue();
				value = value - m_events[i].low_value;
				value *= 127 / (m_events[i].high_value - m_events[i].low_value + 0.00000001);
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
			if (e.GetValue() <= m_events[i].low_value)
				return MIDI_MATCH_OFF;
			if (e.GetValue() >= m_events[i].high_value)
				return MIDI_MATCH_ON;
			continue;
		}
		if (e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_ON && m_events[i].key == e.GetKey() && e.GetValue() >= m_events[i].high_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);
		if (e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_OFF && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);

		if (e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].type == MIDI_M_CTRL_CHANGE && m_events[i].key == e.GetKey())
		{
			if (e.GetValue() <= m_events[i].low_value)
				return MIDI_MATCH_OFF;
			if (e.GetValue() >= m_events[i].high_value)
				return MIDI_MATCH_ON;
			continue;
		}
		if (e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].type == MIDI_M_CTRL_CHANGE_ON && m_events[i].key == e.GetKey() && e.GetValue() >= m_events[i].high_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);
		if (e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].type == MIDI_M_CTRL_CHANGE_OFF && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);

		if (e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN && m_events[i].key == e.GetKey())
		{
			if (e.GetValue() <= m_events[i].low_value)
				return MIDI_MATCH_OFF;
			if (e.GetValue() >= m_events[i].high_value)
				return MIDI_MATCH_ON;
			continue;
		}
		if (e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_ON && m_events[i].key == e.GetKey() && e.GetValue() >= m_events[i].high_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);
		if (e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_OFF && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);

		if (e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN && m_events[i].key == e.GetKey())
		{
			if (e.GetValue() <= m_events[i].low_value)
				return MIDI_MATCH_OFF;
			if (e.GetValue() >= m_events[i].high_value)
				return MIDI_MATCH_ON;
			continue;
		}
		if (e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_ON && m_events[i].key == e.GetKey() && e.GetValue() >= m_events[i].high_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);
		if (e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_OFF && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
			return debounce(e, MIDI_MATCH_CHANGE, i);

		if (e.GetMidiType() == MIDI_PGM_CHANGE && m_events[i].type == MIDI_M_PGM_CHANGE && m_events[i].key == e.GetKey())
		{
			return debounce(e, MIDI_MATCH_CHANGE, i);
		}
		if (e.GetMidiType() == MIDI_SYSEX_JOHANNUS && m_events[i].type == MIDI_M_SYSEX_JOHANNUS && m_events[i].key == e.GetKey())
		{
			return debounce(e, MIDI_MATCH_CHANGE, i);
		}
	}
	return MIDI_MATCH_NONE;
}

GrandOrgueFile* GOrgueMidiReceiver::GetOrganfile()
{
	return m_organfile;
}

void GOrgueMidiReceiver::Assign(const GOrgueMidiReceiverData& data)
{
	*(GOrgueMidiReceiverData*)this = data;
	if (m_organfile)
		m_organfile->Modified();
}
