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

#include "GOrgueMidi.h"

#include "GOrgueEvent.h"
#include "GOrgueMidiListener.h"
#include "GOrgueMidiWXEvent.h"
#include "GOrgueRtHelpers.h"
#include "GOrgueSettings.h"
#include "RtMidi.h"
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/stopwatch.h>
#include <vector>

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

BEGIN_EVENT_TABLE(GOrgueMidi, wxEvtHandler)
	EVT_MIDI(GOrgueMidi::OnMidiEvent)
END_EVENT_TABLE()

GOrgueMidi::GOrgueMidi(GOrgueSettings& settings) :
	m_Settings(settings),
	m_midi_in_device_map(),
	m_midi_in_devices(),
	m_midi_out_device_map(),
	m_midi_out_devices(),
	m_Listeners()
{
	UpdateDevices();
}

void GOrgueMidi::UpdateDevices()
{
	std::vector<RtMidi::Api> apis;
	RtMidi::getCompiledApi(apis);

	try
	{

		for(unsigned j = 0; j < apis.size(); j++)
		{
			RtMidiIn midi_dev(apis[j], "GrandOrgue");
			for (unsigned i = 0; i < midi_dev.getPortCount(); i++)
			{
				wxString name = GOrgueRtHelpers::GetMidiApiPrefix(apis[j]) + wxString::FromAscii(midi_dev.getPortName(i).c_str());
				if (!m_midi_in_device_map.count(name))
				{
					MIDI_IN_DEVICE *t = new MIDI_IN_DEVICE;
					t->midi_in = new RtMidiIn(apis[j], "GrandOrgue");
					t->channel_shift = 0;
					t->midi = this;
					t->rtmidi_port_no = i;
					t->active = false;
					t->name = name;
					t->id = m_Settings.GetMidiMap().GetDeviceByString(t->name);
					t->midi_in->setCallback(&MIDICallback, t);
					m_midi_in_devices.push_back(t);
			
					name.Replace(wxT("\\"), wxT("|"));
					m_midi_in_device_map[name] = m_midi_in_devices.size() - 1;
				}
				else
					m_midi_in_devices[m_midi_in_device_map[name]]->rtmidi_port_no = i;
			}
		}
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}

	try
	{
		for(unsigned j = 0; j < apis.size(); j++)
		{
			RtMidiOut midi_dev(apis[j], "GrandOrgue");
			for (unsigned i = 0; i <  midi_dev.getPortCount(); i++)
			{
				wxString name = GOrgueRtHelpers::GetMidiApiPrefix(apis[j]) + wxString::FromAscii(midi_dev.getPortName(i).c_str());
				if (!m_midi_out_device_map.count(name))
				{
					MIDI_OUT_DEVICE *t = new MIDI_OUT_DEVICE;
					t->midi_out = new RtMidiOut(apis[j], "GrandOrgue");
					t->rtmidi_port_no = i;
					t->active = false;
					t->name = name;
					t->id = m_Settings.GetMidiMap().GetDeviceByString(t->name);
					m_midi_out_devices.push_back(t);
			
					name.Replace(wxT("\\"), wxT("|"));
					m_midi_out_device_map[name] = m_midi_out_devices.size() - 1;
				}
				else
					m_midi_out_devices[m_midi_out_device_map[name]]->rtmidi_port_no = i;
			}
		}
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}
}


GOrgueMidi::~GOrgueMidi()
{
	try
	{
		/* dispose of all midi devices */
		for (unsigned i = 0; i < m_midi_in_devices.size(); i++)
		{
			m_midi_in_devices[i]->active = false;

			if (m_midi_in_devices[i]->midi_in)
				m_midi_in_devices[i]->midi_in->closePort();
			DELETE_AND_NULL(m_midi_in_devices[i]->midi_in);
		}

		for (unsigned i = 0; i < m_midi_out_devices.size(); i++)
		{
			m_midi_out_devices[i]->active = false;

			if (m_midi_out_devices[i]->midi_out)
				m_midi_out_devices[i]->midi_out->closePort();
			DELETE_AND_NULL(m_midi_out_devices[i]->midi_out);
		}
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}

}

void GOrgueMidi::Open()
{
	UpdateDevices();

	for (unsigned i = 0; i < m_midi_in_devices.size(); i++)
	{
		const wxString name = _("GrandOrgue Input");
		MIDI_IN_DEVICE& this_dev = *m_midi_in_devices[i];
		memset(this_dev.bank_msb, 0, sizeof(this_dev.bank_msb));
		memset(this_dev.bank_lsb, 0, sizeof(this_dev.bank_lsb));
		memset(this_dev.rpn_msb, 0, sizeof(this_dev.rpn_msb));
		memset(this_dev.rpn_lsb, 0, sizeof(this_dev.rpn_lsb));
		memset(this_dev.nrpn_msb, 0, sizeof(this_dev.nrpn_msb));
		memset(this_dev.nrpn_lsb, 0, sizeof(this_dev.nrpn_lsb));
		this_dev.rpn = false;
		int channel_shift = m_Settings.GetMidiInDeviceChannelShift(this_dev.name);
		if (m_Settings.GetMidiInState(this_dev.name))
		{
			try
			{
				this_dev.active = false;

				assert(this_dev.midi_in);
				this_dev.channel_shift = channel_shift;
				this_dev.midi_in->closePort();
				this_dev.midi_in->openPort(this_dev.rtmidi_port_no, (const char*)name.fn_str());
				this_dev.active = true;
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				wxLogError(_("RtMidi error: %s"), error.c_str());
			}

		}
		else if (this_dev.active)
		{
			this_dev.active = false;
			this_dev.midi_in->closePort();
		}

	}

	for (unsigned i = 0; i < m_midi_out_devices.size(); i++)
	{
		const wxString name = _("GrandOrgue Output");
		MIDI_OUT_DEVICE& this_dev = *m_midi_out_devices[i];
		if (m_Settings.GetMidiOutState(this_dev.name))
		{
			try
			{
				this_dev.active = false;

				assert(this_dev.midi_out);
				this_dev.midi_out->closePort();
				this_dev.midi_out->openPort(this_dev.rtmidi_port_no, (const char*)name.fn_str());
				this_dev.active = true;
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				wxLogError(_("RtMidi error: %s"), error.c_str());
			}
		}
		else if (this_dev.active)
		{
			this_dev.active = false;
			this_dev.midi_out->closePort();
		}
	}
}

std::map<wxString, int>& GOrgueMidi::GetInDevices()
{
	return m_midi_in_device_map;
}

std::map<wxString, int>& GOrgueMidi::GetOutDevices()
{
	return m_midi_out_device_map;
}

bool GOrgueMidi::HasActiveDevice()
{
	for (unsigned i = 0; i < m_midi_in_devices.size(); i++)
		if (m_midi_in_devices[i]->active)
			return true;

	return false;
}

void GOrgueMidi::ProcessMessage(std::vector<unsigned char>& msg, MIDI_IN_DEVICE* device)
{
	if (!device->active)
		return;

	GOrgueMidiEvent e;
	e.FromMidi(msg);
	if (e.GetMidiType() == MIDI_NONE)
		return;
	e.SetDevice(device->id);
	e.SetTime(wxGetLocalTimeMillis());

	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_BANK_SELECT_MSB)
	{
		device->bank_msb[e.GetChannel() - 1] = e.GetValue();
		return;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_BANK_SELECT_LSB)
	{
		device->bank_lsb[e.GetChannel() - 1] = e.GetValue();
		return;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_RPN_LSB)
	{
		device->rpn_lsb[e.GetChannel() - 1] = e.GetValue();
		device->rpn = true;
		return;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_RPN_MSB)
	{
		device->rpn_msb[e.GetChannel() - 1] = e.GetValue();
		device->rpn = true;
		return;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_NRPN_LSB)
	{
		device->nrpn_lsb[e.GetChannel() - 1] = e.GetValue();
		device->rpn = false;
		return;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_NRPN_MSB)
	{
		device->nrpn_msb[e.GetChannel() - 1] = e.GetValue();
		device->rpn = false;
		return;
	}
	if (e.GetMidiType() == MIDI_PGM_CHANGE)
		e.SetKey(((e.GetKey() - 1) | (device->bank_lsb[e.GetChannel() - 1] << 7) | (device->bank_msb[e.GetChannel() - 1] << 14)) + 1);

	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_DATA_ENTRY)
	{
		if (device->rpn)
		{
			e.SetMidiType(MIDI_RPN);
			e.SetKey((device->rpn_lsb[e.GetChannel() - 1] << 0) | (device->rpn_msb[e.GetChannel() - 1] << 7));
		}
		else
		{
			e.SetMidiType(MIDI_NRPN);
			e.SetKey((device->nrpn_lsb[e.GetChannel() - 1] << 0) | (device->nrpn_msb[e.GetChannel() - 1] << 7));
		}
	}

	/* Compat stuff */
	if (e.GetChannel() != -1)
		e.SetChannel(((e.GetChannel() - 1 + device->channel_shift) & 0x0F) + 1);

	wxMidiEvent event(e);
	AddPendingEvent(event);
}

void GOrgueMidi::OnMidiEvent(wxMidiEvent& event)
{
	GOrgueMidiEvent e = event.GetMidiEvent();
	for(unsigned i = 0; i < m_Listeners.size(); i++)
		if (m_Listeners[i])
			m_Listeners[i]->Send(e);
}

void GOrgueMidi::Send(GOrgueMidiEvent& e)
{
	std::vector<std::vector<unsigned char>> msg;
	e.ToMidi(msg);
	for(unsigned i = 0; i < msg.size(); i++)
	{
		for(unsigned j = 0; j < m_midi_out_devices.size(); j++)
		{
			MIDI_OUT_DEVICE& this_dev = *m_midi_out_devices[j];
			if (!this_dev.active)
				continue;
			if (this_dev.id == e.GetDevice() || e.GetDevice() == 0)
				this_dev.midi_out->sendMessage(&msg[i]);
		}
	}
}

void GOrgueMidi::Register(GOrgueMidiListener* listener)
{
	if (!listener)
		return;
	for(unsigned i = 0; i < m_Listeners.size(); i++)
		if (m_Listeners[i] == listener)
			return;
	for(unsigned i = 0; i < m_Listeners.size(); i++)
		if (!m_Listeners[i])
		{
			m_Listeners[i] = listener;
			return;
		}
	m_Listeners.push_back(listener);
}

void GOrgueMidi::Unregister(GOrgueMidiListener* listener)
{
	for(unsigned i = 0; i < m_Listeners.size(); i++)
		if (m_Listeners[i] == listener)
		{
			m_Listeners[i] = NULL;
		}

}

void GOrgueMidi::MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData)
{
	MIDI_IN_DEVICE* m_dev = (MIDI_IN_DEVICE*)userData;
	m_dev->midi->ProcessMessage(*msg, m_dev);
}
