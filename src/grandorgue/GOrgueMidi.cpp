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
#include "GOrgueMidiRtInPort.h"
#include "GOrgueMidiRtOutPort.h"
#include "GOrgueMidiWXEvent.h"
#include "GOrgueSettings.h"

BEGIN_EVENT_TABLE(GOrgueMidi, wxEvtHandler)
	EVT_MIDI(GOrgueMidi::OnMidiEvent)
END_EVENT_TABLE()

GOrgueMidi::GOrgueMidi(GOrgueSettings& settings) :
	m_Settings(settings),
	m_midi_in_devices(),
	m_midi_out_devices(),
	m_Listeners(),
	m_MidiRecorder(*this),
	m_MidiPlayer(*this)
{
	UpdateDevices();
}

void GOrgueMidi::UpdateDevices()
{
	GOrgueMidiRtInPort::addMissingDevices(this, m_midi_in_devices);
	GOrgueMidiRtOutPort::addMissingDevices(this, m_midi_out_devices);
}


GOrgueMidi::~GOrgueMidi()
{
	m_midi_in_devices.clear();
	m_midi_out_devices.clear();
}

void GOrgueMidi::Open()
{
	UpdateDevices();

	for (unsigned i = 0; i < m_midi_in_devices.size(); i++)
	{
		if (m_Settings.GetMidiInState(m_midi_in_devices[i]->GetName()))
			m_midi_in_devices[i]->Open(m_Settings.GetMidiInDeviceChannelShift(m_midi_in_devices[i]->GetName()));
		else
			m_midi_in_devices[i]->Close();
	}

	for (unsigned i = 0; i < m_midi_out_devices.size(); i++)
	{
		if (m_Settings.GetMidiOutState(m_midi_out_devices[i]->GetName()))
			m_midi_out_devices[i]->Open();
		else
			m_midi_out_devices[i]->Close();
	}

	m_MidiRecorder.SetOutputDevice(GetMidiMap().GetDeviceByString(m_Settings.GetMidiRecorderOutputDevice()));
}

std::vector<wxString> GOrgueMidi::GetInDevices()
{
	std::vector<wxString> list;
	for(unsigned i = 0; i < m_midi_in_devices.size(); i++)
		if (m_midi_in_devices[i])
			list.push_back(m_midi_in_devices[i]->GetName());
	return list;
}

std::vector<wxString> GOrgueMidi::GetOutDevices()
{
	std::vector<wxString> list;
	for(unsigned i = 0; i < m_midi_out_devices.size(); i++)
		if (m_midi_out_devices[i])
			list.push_back(m_midi_out_devices[i]->GetName());
	return list;
}

bool GOrgueMidi::HasActiveDevice()
{
	for (unsigned i = 0; i < m_midi_in_devices.size(); i++)
		if (m_midi_in_devices[i]->IsActive())
			return true;

	return false;
}

void GOrgueMidi::Recv(const GOrgueMidiEvent& e)
{
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

void GOrgueMidi::Send(const GOrgueMidiEvent& e)
{
	for(unsigned j = 0; j < m_midi_out_devices.size(); j++)
		m_midi_out_devices[j]->Send(e);
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

GOrgueMidiRecorder& GOrgueMidi::GetMidiRecorder()
{
	return m_MidiRecorder;
}

GOrgueMidiPlayer& GOrgueMidi::GetMidiPlayer()
{
	return m_MidiPlayer;
}

GOrgueMidiMap& GOrgueMidi::GetMidiMap()
{
	return m_Settings.GetMidiMap();
}
