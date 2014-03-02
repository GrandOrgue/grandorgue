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

#include "GOrgueMidiSender.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueMidiMap.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

GOrgueMidiSender::GOrgueMidiSender(GrandOrgueFile* organfile, MIDI_SENDER_TYPE type) :
	GOrgueMidiSenderData(type),
	m_organfile(organfile),
	m_ElementID(-1)
{
}

GOrgueMidiSender::~GOrgueMidiSender()
{
}

const struct IniFileEnumEntry GOrgueMidiSender::m_MidiTypes[] = {
	{ wxT("Note"), MIDI_S_NOTE },
	{ wxT("NoteNoVelocity"), MIDI_S_NOTE_NO_VELOCITY },
	{ wxT("ControlChange"), MIDI_S_CTRL },
	{ wxT("RPN"), MIDI_S_RPN },
	{ wxT("NRPN"), MIDI_S_NRPN },
	{ wxT("ProgramOn"), MIDI_S_PGM_ON },
	{ wxT("ProgramOff"), MIDI_S_PGM_OFF },
	{ wxT("NoteOn"), MIDI_S_NOTE_ON },
	{ wxT("NoteOff"), MIDI_S_NOTE_OFF },
	{ wxT("ControlOn"), MIDI_S_CTRL_ON },
	{ wxT("ControlOff"), MIDI_S_CTRL_OFF },
	{ wxT("RPNOn"), MIDI_S_RPN_ON },
	{ wxT("RPNOff"), MIDI_S_RPN_OFF },
	{ wxT("NRPNOn"), MIDI_S_NRPN_ON },
	{ wxT("NRPNOff"), MIDI_S_NRPN_OFF },
	{ wxT("HWLCD"), MIDI_S_HW_LCD },
	{ wxT("HWString"), MIDI_S_HW_STRING },
};

void GOrgueMidiSender::SetElementID(int id)
{
	m_ElementID = id;
}

void GOrgueMidiSender::Load(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map)
{
	m_events.resize(0);

	int event_cnt = cfg.ReadInteger(CMBSetting, group, wxT("NumberOfMIDISendEvents"), 0, 255, false);

	m_events.resize(event_cnt);
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		m_events[i].device = map.GetDeviceByString(cfg.ReadString(CMBSetting, group, wxString::Format(wxT("MIDISendDevice%03d"), i + 1), false));
		m_events[i].type = (midi_send_message_type)cfg.ReadEnum(CMBSetting, group, wxString::Format(wxT("MIDISendEventType%03d"), i + 1), m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
		if (HasChannel(m_events[i].type))
			m_events[i].channel = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDISendChannel%03d"), i + 1), 1, 16);
		if (HasKey(m_events[i].type))
			m_events[i].key = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDISendKey%03d"), i + 1), 0, 0x200000);

		if (HasLowValue(m_events[i].type))
			m_events[i].low_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDISendLowValue%03d"), i + 1), 0, 0x7f, false, 0);

		if (HasHighValue(m_events[i].type))
			m_events[i].high_value = cfg.ReadInteger(CMBSetting, group, wxString::Format(wxT("MIDISendHighValue%03d"), i + 1), 0, 0x7f, false, 0x7f);
	}
}

void GOrgueMidiSender::Save(GOrgueConfigWriter& cfg, wxString group, GOrgueMidiMap& map)
{
	cfg.WriteInteger(group, wxT("NumberOfMIDISendEvents"), m_events.size());
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		cfg.WriteString(group, wxString::Format(wxT("MIDISendDevice%03d"), i + 1), map.GetDeviceByID(m_events[i].device));
		cfg.WriteEnum(group, wxString::Format(wxT("MIDISendEventType%03d"), i + 1), m_events[i].type, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
		if (HasChannel(m_events[i].type))
			cfg.WriteInteger(group, wxString::Format(wxT("MIDISendChannel%03d"), i + 1), m_events[i].channel);
		if (HasKey(m_events[i].type))
			cfg.WriteInteger(group, wxString::Format(wxT("MIDISendKey%03d"), i + 1), m_events[i].key);

		if (HasLowValue(m_events[i].type))
			cfg.WriteInteger(group, wxString::Format(wxT("MIDISendLowValue%03d"), i + 1), m_events[i].low_value);

		if (HasHighValue(m_events[i].type))
			cfg.WriteInteger(group, wxString::Format(wxT("MIDISendHighValue%03d"), i + 1), m_events[i].high_value);
	}
}

bool GOrgueMidiSender::HasChannel(midi_send_message_type type)
{
	if (type == MIDI_S_NOTE ||
	    type == MIDI_S_NOTE_NO_VELOCITY ||
	    type == MIDI_S_CTRL ||
	    type == MIDI_S_RPN ||
	    type == MIDI_S_NRPN ||
	    type == MIDI_S_PGM_ON ||
	    type == MIDI_S_PGM_OFF ||
	    type == MIDI_S_NOTE_ON ||
	    type == MIDI_S_NOTE_OFF ||
	    type == MIDI_S_CTRL_ON ||
	    type == MIDI_S_CTRL_OFF ||
	    type == MIDI_S_RPN_ON ||
	    type == MIDI_S_RPN_OFF ||
	    type == MIDI_S_NRPN_ON ||
	    type == MIDI_S_NRPN_OFF)
		return true;

	return false;
}

bool GOrgueMidiSender::HasKey(midi_send_message_type type)
{
	if (m_type == MIDI_SEND_MANUAL)
		return false;

	if (type == MIDI_S_NOTE ||
	    type == MIDI_S_NOTE_NO_VELOCITY ||
	    type == MIDI_S_CTRL ||
	    type == MIDI_S_RPN ||
	    type == MIDI_S_NRPN ||
	    type == MIDI_S_PGM_ON ||
	    type == MIDI_S_PGM_OFF ||
	    type == MIDI_S_NOTE_ON ||
	    type == MIDI_S_NOTE_OFF ||
	    type == MIDI_S_CTRL_ON ||
	    type == MIDI_S_CTRL_OFF ||
	    type == MIDI_S_RPN_ON ||
	    type == MIDI_S_RPN_OFF ||
	    type == MIDI_S_NRPN_ON ||
	    type == MIDI_S_NRPN_OFF ||
	    type == MIDI_S_HW_STRING ||
	    type == MIDI_S_HW_LCD)
		return true;

	return false;
}

bool GOrgueMidiSender::HasLowValue(midi_send_message_type type)
{
	if (type == MIDI_S_NOTE_OFF ||
	    type == MIDI_S_CTRL_OFF ||
	    type == MIDI_S_RPN_OFF ||
	    type == MIDI_S_NRPN_OFF ||
	    type == MIDI_S_NOTE ||
	    type == MIDI_S_NOTE_NO_VELOCITY ||
	    type == MIDI_S_RPN ||
	    type == MIDI_S_NRPN ||
	    type == MIDI_S_CTRL ||
	    type == MIDI_S_HW_LCD)
		return true;
	return false;
}

bool GOrgueMidiSender::HasHighValue(midi_send_message_type type)
{
	if (type == MIDI_S_NOTE_ON ||
	    type == MIDI_S_CTRL_ON ||
	    type == MIDI_S_RPN_ON ||
	    type == MIDI_S_NRPN_ON ||
	    type == MIDI_S_NOTE ||
	    type == MIDI_S_NOTE_NO_VELOCITY ||
	    type == MIDI_S_RPN ||
	    type == MIDI_S_NRPN ||
	    type == MIDI_S_CTRL)
		return true;
	return false;
}

void GOrgueMidiSender::SetDisplay(bool state)
{
	if (m_ElementID != -1)
	{
		GOrgueMidiEvent e;
		e.SetMidiType(MIDI_NRPN);
		e.SetDevice(m_ElementID);
		e.SetValue(state ? 0x7F : 0x00);
		m_organfile->SendMidiRecorderMessage(e);
	}

	for(unsigned i = 0; i < m_events.size(); i++)
	{
		if (m_events[i].type == MIDI_S_NOTE)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NOTE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_CTRL)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_CTRL_CHANGE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_RPN)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_RPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_NRPN)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NRPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_PGM_ON && state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_PGM_CHANGE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_PGM_OFF && !state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_PGM_CHANGE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_NOTE_ON && state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NOTE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].high_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_NOTE_OFF && !state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NOTE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_CTRL_ON && state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_CTRL_CHANGE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].high_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_CTRL_OFF && !state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_CTRL_CHANGE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_RPN_ON && state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_RPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].high_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_RPN_OFF && !state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_RPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_NRPN_ON && state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NRPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].high_value);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_NRPN_OFF && !state)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NRPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			e.SetValue(m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
	}
}

void GOrgueMidiSender::ResetKey()
{
	if (m_ElementID != -1)
	{
		GOrgueMidiEvent e;
		e.SetMidiType(MIDI_CTRL_CHANGE);
		e.SetDevice(m_ElementID);
		e.SetKey(MIDI_CTRL_NOTES_OFF);
		e.SetValue(0);
		m_organfile->SendMidiRecorderMessage(e);
	}

	for(unsigned i = 0; i < m_events.size(); i++)
	{
		if (m_events[i].type == MIDI_S_NOTE || m_events[i].type == MIDI_S_NOTE_NO_VELOCITY)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_CTRL_CHANGE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(MIDI_CTRL_NOTES_OFF);
			e.SetValue(0);
			m_organfile->SendMidiMessage(e);
		}
	}
}

void GOrgueMidiSender::SetKey(unsigned key, unsigned velocity)
{
	if (m_ElementID != -1)
	{
		GOrgueMidiEvent e;
		e.SetMidiType(MIDI_NOTE);
		e.SetDevice(m_ElementID);
		e.SetKey(key & 0x7F);
		e.SetValue(velocity & 0x7F);
		m_organfile->SendMidiRecorderMessage(e);
	}

	for(unsigned i = 0; i < m_events.size(); i++)
	{
		if (m_events[i].type == MIDI_S_NOTE)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NOTE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(key);
			e.SetValue(m_events[i].low_value + velocity * (m_events[i].high_value - m_events[i].low_value));
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_NOTE_NO_VELOCITY)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NOTE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(key);
			e.SetValue(velocity ? m_events[i].high_value : m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
	}
}

void GOrgueMidiSender::SetValue(unsigned value)
{
	if (m_ElementID != -1)
	{
		GOrgueMidiEvent e;
		e.SetMidiType(MIDI_NRPN);
		e.SetDevice(m_ElementID);
		e.SetValue(value & 0x7F);
		m_organfile->SendMidiRecorderMessage(e);
	}

	for(unsigned i = 0; i < m_events.size(); i++)
	{
		if (m_events[i].type == MIDI_S_CTRL)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_CTRL_CHANGE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			unsigned val = m_events[i].low_value + ((m_events[i].high_value - m_events[i].low_value) * value) / 0x7f;
			if (val < 0)
				val = 0;
			if (val > 0x7f)
				val = 0x7f;
			e.SetValue(val);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_RPN)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_RPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			unsigned val = m_events[i].low_value + ((m_events[i].high_value - m_events[i].low_value) * value) / 0x7f;
			if (val < 0)
				val = 0;
			if (val > 0x7f)
				val = 0x7f;
			e.SetValue(val);
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_NRPN)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NRPN);
			e.SetChannel(m_events[i].channel);
			e.SetKey(m_events[i].key);
			unsigned val = m_events[i].low_value + ((m_events[i].high_value - m_events[i].low_value) * value) / 0x7f;
			if (val < 0)
				val = 0;
			if (val > 0x7f)
				val = 0x7f;
			e.SetValue(val);
			m_organfile->SendMidiMessage(e);
		}
	}
}

void GOrgueMidiSender::SetLabel(wxString text)
{
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		if (m_events[i].type == MIDI_S_HW_LCD)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_SYSEX_HW_LCD);
			e.SetChannel(m_events[i].low_value);
			e.SetKey(m_events[i].key);
			e.SetValue(m_organfile->GetSettings().GetMidiMap().GetElementByString(text));
			m_organfile->SendMidiMessage(e);
		}
		if (m_events[i].type == MIDI_S_HW_STRING)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_SYSEX_HW_STRING);
			e.SetKey(m_events[i].key);
			e.SetValue(m_organfile->GetSettings().GetMidiMap().GetElementByString(text));
			m_organfile->SendMidiMessage(e);
		}
	}
}

void GOrgueMidiSender::Assign(const GOrgueMidiSenderData& data)
{
	*(GOrgueMidiSenderData*)this = data;
	if (m_organfile)
		m_organfile->Modified();
}
