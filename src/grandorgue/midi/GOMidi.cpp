/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidi.h"

#include "GOEvent.h"
#include "GOMidiListener.h"
#include "midi/GOMidiWXEvent.h"
#include "settings/GOSettings.h"

BEGIN_EVENT_TABLE(GOMidi, wxEvtHandler)
	EVT_MIDI(GOMidi::OnMidiEvent)
END_EVENT_TABLE()

GOMidi::GOMidi(GOSettings& settings) :
	m_Settings(settings),
	m_midi_in_devices(),
	m_midi_out_devices(),
	m_Listeners()
{
}

void GOMidi::UpdateDevices(const GOPortsConfig& portsConfig)
{
  m_midi_in_devices.clear();
  m_MidiFactory.addMissingInDevices(this, portsConfig, m_midi_in_devices);
  m_midi_out_devices.clear();
  m_MidiFactory.addMissingOutDevices(this, portsConfig, m_midi_out_devices);
}


GOMidi::~GOMidi()
{
	m_midi_in_devices.clear();
	m_midi_out_devices.clear();
}

void GOMidi::Open()
{
	UpdateDevices(m_Settings.GetMidiPortsConfig());

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
}

std::vector<wxString> GOMidi::GetInDevices()
{
	std::vector<wxString> list;
	for(unsigned i = 0; i < m_midi_in_devices.size(); i++)
		if (m_midi_in_devices[i])
			list.push_back(m_midi_in_devices[i]->GetName());
	return list;
}

std::vector<wxString> GOMidi::GetOutDevices()
{
	std::vector<wxString> list;
	for(unsigned i = 0; i < m_midi_out_devices.size(); i++)
		if (m_midi_out_devices[i])
			list.push_back(m_midi_out_devices[i]->GetName());
	return list;
}

bool GOMidi::HasActiveDevice()
{
	for (unsigned i = 0; i < m_midi_in_devices.size(); i++)
		if (m_midi_in_devices[i]->IsActive())
			return true;

	return false;
}

void GOMidi::Recv(const GOMidiEvent& e)
{
	wxMidiEvent event(e);
	AddPendingEvent(event);
}

void GOMidi::OnMidiEvent(wxMidiEvent& event)
{
	GOMidiEvent e = event.GetMidiEvent();
	for(unsigned i = 0; i < m_Listeners.size(); i++)
		if (m_Listeners[i])
			m_Listeners[i]->Send(e);
}

void GOMidi::Send(const GOMidiEvent& e)
{
	for(unsigned j = 0; j < m_midi_out_devices.size(); j++)
		m_midi_out_devices[j]->Send(e);
}

void GOMidi::Register(GOMidiListener* listener)
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

void GOMidi::Unregister(GOMidiListener* listener)
{
	for(unsigned i = 0; i < m_Listeners.size(); i++)
		if (m_Listeners[i] == listener)
		{
			m_Listeners[i] = NULL;
		}

}

GOMidiMap& GOMidi::GetMidiMap()
{
	return m_Settings.GetMidiMap();
}
