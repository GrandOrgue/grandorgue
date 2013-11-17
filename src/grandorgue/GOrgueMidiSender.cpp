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

#include "GOrgueMidiSender.h"
#include "GOrgueMidiEvent.h"
#include "GrandOrgueFile.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

GOrgueMidiSender::GOrgueMidiSender(GrandOrgueFile* organfile, MIDI_SENDER_TYPE type) :
	GOrgueMidiSenderData(type),
	m_organfile(organfile)
{
}

GOrgueMidiSender::~GOrgueMidiSender()
{
}

const struct IniFileEnumEntry GOrgueMidiSender::m_MidiTypes[] = {
	{ wxT("Note"), MIDI_S_NOTE },
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
};

void GOrgueMidiSender::Load(GOrgueConfigReader& cfg, wxString group)
{
	wxString buffer;
	m_events.resize(0);

	int event_cnt = cfg.ReadInteger(CMBSetting, group, wxT("NumberOfMIDISendEvents"), 0, 255, false);

	m_events.resize(event_cnt);
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		buffer.Printf(wxT("MIDISendDevice%03d"), i + 1);
		m_events[i].device = cfg.ReadString(CMBSetting, group, buffer, 100, false);
		buffer.Printf(wxT("MIDISendChannel%03d"), i + 1);
		m_events[i].channel = cfg.ReadInteger(CMBSetting, group, buffer, 1, 16);
		buffer.Printf(wxT("MIDISendEventType%03d"), i + 1);
		m_events[i].type = (midi_send_message_type)cfg.ReadEnum(CMBSetting, group, buffer, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
		if (m_type != MIDI_SEND_MANUAL)
		{
			buffer.Printf(wxT("MIDISendKey%03d"), i + 1);
			m_events[i].key = cfg.ReadInteger(CMBSetting, group, buffer, 0, 0x200000);
		}
		if (m_events[i].type == MIDI_S_NOTE_OFF ||
		    m_events[i].type == MIDI_S_CTRL_OFF ||
		    m_events[i].type == MIDI_S_RPN_OFF ||
		    m_events[i].type == MIDI_S_NRPN_OFF ||
		    m_events[i].type == MIDI_S_PGM_OFF ||
		    m_events[i].type == MIDI_S_NOTE ||
		    m_events[i].type == MIDI_S_RPN ||
		    m_events[i].type == MIDI_S_NRPN ||
		    m_events[i].type == MIDI_S_CTRL)
		{
			buffer.Printf(wxT("MIDISendLowValue%03d"), i + 1);
			m_events[i].low_value = cfg.ReadInteger(CMBSetting, group, buffer, 0, 0x7f, false, 0);
		}
		if (m_events[i].type == MIDI_S_NOTE_ON ||
		    m_events[i].type == MIDI_S_CTRL_ON ||
		    m_events[i].type == MIDI_S_RPN_ON ||
		    m_events[i].type == MIDI_S_NRPN_ON ||
		    m_events[i].type == MIDI_S_PGM_ON ||
		    m_events[i].type == MIDI_S_NOTE ||
		    m_events[i].type == MIDI_S_RPN ||
		    m_events[i].type == MIDI_S_NRPN ||
		    m_events[i].type == MIDI_S_CTRL)
		{
			buffer.Printf(wxT("MIDISendHighValue%03d"), i + 1);
			m_events[i].high_value = cfg.ReadInteger(CMBSetting, group, buffer, 0, 0x7f, false, 0x7f);
		}
	}
}

void GOrgueMidiSender::Save(GOrgueConfigWriter& cfg, wxString group)
{
	wxString buffer;

	cfg.WriteInteger(group, wxT("NumberOfMIDISendEvents"), m_events.size());
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		buffer.Printf(wxT("MIDISendDevice%03d"), i + 1);
		cfg.WriteString(group, buffer, m_events[i].device);
		buffer.Printf(wxT("MIDISendChannel%03d"), i + 1);
		cfg.WriteInteger(group, buffer, m_events[i].channel);
		buffer.Printf(wxT("MIDISendEventType%03d"), i + 1);
		cfg.WriteEnum(group, buffer, m_events[i].type, m_MidiTypes, sizeof(m_MidiTypes)/sizeof(m_MidiTypes[0]));
		if (m_type != MIDI_SEND_MANUAL)
		{
			buffer.Printf(wxT("MIDISendKey%03d"), i + 1);
			cfg.WriteInteger(group, buffer, m_events[i].key);
		}
		if (m_events[i].type == MIDI_S_NOTE_OFF ||
		    m_events[i].type == MIDI_S_CTRL_OFF ||
		    m_events[i].type == MIDI_S_RPN_OFF ||
		    m_events[i].type == MIDI_S_NRPN_OFF ||
		    m_events[i].type == MIDI_S_PGM_OFF ||
		    m_events[i].type == MIDI_S_NOTE ||
		    m_events[i].type == MIDI_S_RPN ||
		    m_events[i].type == MIDI_S_NRPN ||
		    m_events[i].type == MIDI_S_CTRL)
		{
			buffer.Printf(wxT("MIDISendLowValue%03d"), i + 1);
			cfg.WriteInteger(group, buffer, m_events[i].low_value);
		}
		if (m_events[i].type == MIDI_S_NOTE_ON ||
		    m_events[i].type == MIDI_S_CTRL_ON ||
		    m_events[i].type == MIDI_S_RPN_ON ||
		    m_events[i].type == MIDI_S_NRPN_ON ||
		    m_events[i].type == MIDI_S_PGM_ON ||
		    m_events[i].type == MIDI_S_NOTE ||
		    m_events[i].type == MIDI_S_RPN ||
		    m_events[i].type == MIDI_S_NRPN ||
		    m_events[i].type == MIDI_S_CTRL)
		{
			buffer.Printf(wxT("MIDISendHighValue%03d"), i + 1);
			cfg.WriteInteger(group, buffer, m_events[i].high_value);
		}
	}
}

void GOrgueMidiSender::SetDisplay(bool state)
{
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

void GOrgueMidiSender::SetKey(unsigned key, bool state)
{
	for(unsigned i = 0; i < m_events.size(); i++)
	{
		if (m_events[i].type == MIDI_S_NOTE)
		{
			GOrgueMidiEvent e;
			e.SetDevice(m_events[i].device);
			e.SetMidiType(MIDI_NOTE);
			e.SetChannel(m_events[i].channel);
			e.SetKey(key);
			e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
			m_organfile->SendMidiMessage(e);
		}
	}
}

void GOrgueMidiSender::SetValue(unsigned value)
{
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
}

void GOrgueMidiSender::Assign(const GOrgueMidiSenderData& data)
{
	*(GOrgueMidiSenderData*)this = data;
	if (m_organfile)
		m_organfile->Modified();
}
