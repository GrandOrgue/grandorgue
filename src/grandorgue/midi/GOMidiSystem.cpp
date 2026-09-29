/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSystem.h"

#include <algorithm>

#include <wx/log.h>
#include <wx/timer.h>

#include "GOEvent.h"
#include "GOMidiListener.h"
#include "config/GOConfig.h"
#include "config/GOMidiDeviceConfig.h"
#include "midi/events/GOMidiWxEvent.h"
#include "ports/GOMidiInPort.h"
#include "ports/GOMidiOutPort.h"
#include "ports/GOMidiPortFactory.h"

static constexpr int ID_MIDI_DISCOVERY_TIMER = wxID_HIGHEST + 1;
static constexpr int MIDI_DISCOVERY_INTERVAL_MS = 1000;

BEGIN_EVENT_TABLE(GOMidiSystem, wxEvtHandler)
EVT_MIDI(GOMidiSystem::OnMidiEvent)
EVT_TIMER(ID_MIDI_DISCOVERY_TIMER, GOMidiSystem::OnDiscoveryTimer)
END_EVENT_TABLE()

GOMidiSystem::GOMidiSystem(GOConfig &config)
  : m_config(config),
    m_MidiMap(config.GetMidiMap()),
    m_pDiscoveryTimer(new wxTimer(this, ID_MIDI_DISCOVERY_TIMER)) {}

void GOMidiSystem::UpdateDevices(const GOPortsConfig &portsConfig) {
  m_MidiFactory.addMissingInDevices(this, portsConfig, m_midi_in_devices);
  m_MidiFactory.addMissingOutDevices(this, portsConfig, m_midi_out_devices);
}

GOMidiSystem::~GOMidiSystem() {
  if (m_pDiscoveryTimer->IsRunning())
    m_pDiscoveryTimer->Stop();
  delete m_pDiscoveryTimer;
  m_midi_in_devices.clear();
  m_midi_out_devices.clear();

  GOMidiPortFactory::terminate();
}

void GOMidiSystem::Open() {
  if (m_pDiscoveryTimer->IsRunning())
    m_pDiscoveryTimer->Stop();
  OpenDevices(false);
  if (HasPendingDevices())
    m_pDiscoveryTimer->Start(MIDI_DISCOVERY_INTERVAL_MS);
}

bool GOMidiSystem::HasPendingDevices() const {
  return HasPendingDevices(m_config.m_MidiIn, m_midi_in_devices)
    || HasPendingDevices(m_config.m_MidiOut, m_midi_out_devices);
}

bool GOMidiSystem::HasPendingDevices(
  const GOMidiDeviceConfigList &configs,
  const ptr_vector<GOMidiPort> &ports) const {
  const GOPortsConfig &portsConfig = m_config.GetMidiPortsConfig();

  return std::ranges::any_of(
    configs,
    [this, &ports, &portsConfig](const GOMidiDeviceConfig *config) {
      if (
        !config->m_IsEnabled
        || !portsConfig.IsEnabled(
          config->GetPortName(), config->GetApiName()))
        return false;

      const unsigned id
        = m_MidiMap.GetDeviceIdByLogicalName(config->GetLogicalName());
      return !std::ranges::any_of(
        ports,
        [id](const GOMidiPort *port) {
          return port->IsActive() && port->GetID() == id;
        });
    });
}

void GOMidiSystem::DiscoverDevices() {
  // Open intent stays active during backend reconnection. Discovery opens
  // only inactive ports, so an active port is not reopened even if
  // enumeration now reports a different native address for the port.
  if (HasPendingDevices()) {
    // A newly arriving driver may not be ready yet. Retry without repeated
    // error dialogs; an explicit Open() continues to report errors normally.
    wxLogNull quiet;

    OpenDevices(true);
  }
}

void GOMidiSystem::OnDiscoveryTimer(wxTimerEvent &WXUNUSED(event)) {
  DiscoverDevices();
  if (!HasPendingDevices())
    m_pDiscoveryTimer->Stop();
}

void GOMidiSystem::OpenDevices(bool onlyInactive) {
  const bool isToAutoAdd = !onlyInactive && m_config.IsToAutoAddMidi();
  const GOPortsConfig &portsConfig(m_config.GetMidiPortsConfig());
  GOMidiDeviceConfigList &midiIn = m_config.m_MidiIn;

  UpdateDevices(portsConfig);

  for (GOMidiPort *pPort : m_midi_in_devices) {
    if (!onlyInactive || !pPort->IsActive()) {
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
      if (pDevConf && pDevConf->m_IsEnabled) {
        const bool opened = ((GOMidiInPort *)pPort)
                              ->Open(
                                m_MidiMap.EnsureLogicalName(
                                  pDevConf->GetLogicalName()),
                                pDevConf->m_ChannelShift);

        // If opening raced device removal, allow the next scan to match
        // its replacement instead of retaining the failed port's physical
        // name.
        if (onlyInactive && !opened)
          pDevConf->SetPhysicalName(wxEmptyString);
      } else
        pPort->Close();
    }
  }

  for (GOMidiPort *pPort : m_midi_out_devices) {
    if (!onlyInactive || !pPort->IsActive()) {
      const wxString &portName = pPort->GetPortName();
      const wxString &apiName = pPort->GetApiName();
      GOMidiDeviceConfig *devConf = NULL;

      if (
        pPort->IsToUse() && portsConfig.IsEnabled(portName, apiName)
        && (devConf = m_config.m_MidiOut.FindByPhysicalName(
          pPort->GetName(), portName, apiName))
        && devConf->m_IsEnabled) {
        const bool opened
          = pPort->Open(
            m_MidiMap.EnsureLogicalName(devConf->GetLogicalName()));

        if (onlyInactive && !opened)
          devConf->SetPhysicalName(wxEmptyString);
      } else
        pPort->Close();
    }
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
