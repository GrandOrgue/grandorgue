/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSystem.h"

#include "GOEvent.h"
#include "GOMidiListener.h"
#include "config/GOConfig.h"
#include "config/GOMidiDeviceConfig.h"
#include "midi/events/GOMidiWxEvent.h"
#include "ports/GOMidiInPort.h"
#include "ports/GOMidiOutPort.h"

BEGIN_EVENT_TABLE(GOMidiSystem, wxEvtHandler)
EVT_MIDI(GOMidiSystem::OnMidiEvent)
END_EVENT_TABLE()

GOMidiSystem::GOMidiSystem(GOConfig &config)
  : m_config(config), m_MidiMap(config.GetMidiMap()) {}

void GOMidiSystem::UpdateDevices(const GOPortsConfig &portsConfig) {
  m_MidiFactory.addMissingInDevices(this, portsConfig, m_midi_in_devices);
  m_MidiFactory.addMissingOutDevices(this, portsConfig, m_midi_out_devices);
}

GOMidiSystem::~GOMidiSystem() {
  m_midi_in_devices.clear();
  m_midi_out_devices.clear();
}

void GOMidiSystem::Open() {
  const bool isToAutoAdd = m_config.IsToAutoAddMidi();
  const GOPortsConfig &portsConfig(m_config.GetMidiPortsConfig());
  GOMidiDeviceConfigList &midiIn = m_config.m_MidiIn;

  UpdateDevices(portsConfig);

  for (GOMidiPort *pPort : m_midi_in_devices) {
    const wxString &portName = pPort->GetPortName();
    const wxString &apiName = pPort->GetApiName();
    GOMidiDeviceConfig *pDevConf = NULL;

    if (pPort->IsToUse() && portsConfig.IsEnabled(portName, apiName)) {
      const wxString &physicalName = pPort->GetName();

      pDevConf = midiIn.FindByPhysicalName(physicalName, portName, apiName);
      if (!pDevConf && isToAutoAdd)
        pDevConf = midiIn.Append(
          pPort->GetDefaultLogicalName(),
          pPort->GetDefaultRegEx(),
          portName,
          apiName,
          true,
          physicalName);
    }
    if (pDevConf && pDevConf->m_IsEnabled)
      ((GOMidiInPort *)pPort)
        ->Open(
          m_MidiMap.EnsureLogicalName(pDevConf->GetLogicalName()),
          pDevConf->m_ChannelShift);
    else
      pPort->Close();
  }

  for (GOMidiPort *pPort : m_midi_out_devices) {
    const wxString &portName = pPort->GetPortName();
    const wxString &apiName = pPort->GetApiName();
    GOMidiDeviceConfig *devConf;

    if (
      pPort->IsToUse() && portsConfig.IsEnabled(portName, apiName)
      && (devConf = m_config.m_MidiOut.FindByPhysicalName(pPort->GetName(), portName, apiName))
      && devConf->m_IsEnabled)
      pPort->Open(m_MidiMap.EnsureLogicalName(devConf->GetLogicalName()));
    else
      pPort->Close();
  }
}

bool GOMidiSystem::HasActiveDevice() {
  for (unsigned i = 0; i < m_midi_in_devices.size(); i++)
    if (m_midi_in_devices[i]->IsActive())
      return true;

  return false;
}

void GOMidiSystem::Recv(const GOMidiEvent &e) {
  GOMidiWxEvent event(e);
  AddPendingEvent(event);
}

void GOMidiSystem::PlayEvent(const GOMidiEvent &e) {
  for (unsigned i = 0; i < m_Listeners.size(); i++)
    if (m_Listeners[i])
      m_Listeners[i]->Send(e);
}

void GOMidiSystem::OnMidiEvent(GOMidiWxEvent &e) {
  PlayEvent(e.GetMidiEvent());
}

void GOMidiSystem::Send(const GOMidiEvent &e) {
  for (unsigned j = 0; j < m_midi_out_devices.size(); j++)
    ((GOMidiOutPort *)m_midi_out_devices[j])->Send(e);
}

void GOMidiSystem::Register(GOMidiListener *listener) {
  if (!listener)
    return;
  for (unsigned i = 0; i < m_Listeners.size(); i++)
    if (m_Listeners[i] == listener)
      return;
  for (unsigned i = 0; i < m_Listeners.size(); i++)
    if (!m_Listeners[i]) {
      m_Listeners[i] = listener;
      return;
    }
  m_Listeners.push_back(listener);
}

void GOMidiSystem::Unregister(GOMidiListener *listener) {
  for (unsigned i = 0; i < m_Listeners.size(); i++)
    if (m_Listeners[i] == listener) {
      m_Listeners[i] = NULL;
    }
}
