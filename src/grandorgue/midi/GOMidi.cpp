/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidi.h"

#include "midi/GOMidiWXEvent.h"
#include "ports/GOMidiInPort.h"
#include "ports/GOMidiOutPort.h"
#include "settings/GOMidiDeviceConfig.h"
#include "settings/GOSettings.h"

#include "GOEvent.h"
#include "GOMidiListener.h"

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
  m_MidiFactory.addMissingInDevices(this, portsConfig, m_midi_in_devices);
  m_MidiFactory.addMissingOutDevices(this, portsConfig, m_midi_out_devices);
}


GOMidi::~GOMidi()
{
	m_midi_in_devices.clear();
	m_midi_out_devices.clear();
}

void GOMidi::Open()
{
	const bool isToAutoAdd = m_Settings.IsToAutoAddMidi();
	const GOPortsConfig& portsConfig(m_Settings.GetMidiPortsConfig());
	GOMidiDeviceConfigList& midiIn = m_Settings.m_MidiIn;

	UpdateDevices(portsConfig);

	for (GOMidiPort* pPort : m_midi_in_devices)
	{
	  GOMidiDeviceConfig* pDevConf = NULL;

	  if (
	    pPort->IsToUse()
	    && portsConfig.IsEnabled(pPort->GetPortName(), pPort->GetApiName())
	  )
	  {
	    const wxString& physicalName = pPort->GetName();
	    
	    pDevConf = midiIn.FindByPhysicalName(physicalName);
	    if (! pDevConf && isToAutoAdd)
	      pDevConf = midiIn.Append(
	        pPort->GetDefaultLogicalName(),
	        pPort->GetDefaultRegEx(),
	        true,
	        physicalName
	      );
	  }
	  if (pDevConf && pDevConf->m_IsEnabled)
	    ((GOMidiInPort *) pPort)->Open(pDevConf->m_ChannelShift);
	  else
	    pPort->Close();
	}

	for (GOMidiPort* pPort : m_midi_out_devices)
	{
	  GOMidiDeviceConfig* devConf;

	  if (
	    pPort->IsToUse()
	    && portsConfig.IsEnabled(pPort->GetPortName(), pPort->GetApiName())
	    && (devConf = m_Settings.m_MidiOut.FindByPhysicalName(pPort->GetName()))
	    && devConf->m_IsEnabled
	  )
	    pPort->Open();
	  else
	    pPort->Close();
	}
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
		((GOMidiOutPort*) m_midi_out_devices[j])->Send(e);
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
