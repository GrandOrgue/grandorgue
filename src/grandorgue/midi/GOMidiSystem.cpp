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
    o_DiscoveryTimer(new wxTimer(this, ID_MIDI_DISCOVERY_TIMER)) {}

void GOMidiSystem::UpdateDevices(const GOPortsConfig &portsConfig) {
  m_MidiFactory.addMissingInDevices(this, portsConfig, m_midi_in_devices);
  m_MidiFactory.addMissingOutDevices(this, portsConfig, m_midi_out_devices);
}

GOMidiSystem::~GOMidiSystem() {
  EnsureDiscoveryTimerStopped();
  m_midi_in_devices.clear();
  m_midi_out_devices.clear();

  GOMidiPortFactory::terminate();
}

void GOMidiSystem::EnsureDiscoveryTimerStopped() {
  if (o_DiscoveryTimer->IsRunning())
    o_DiscoveryTimer->Stop();
}

void GOMidiSystem::Open() {
  EnsureDiscoveryTimerStopped();
  OpenDevices(true);
  if (HasPendingDevices())
    o_DiscoveryTimer->Start(MIDI_DISCOVERY_INTERVAL_MS);
}

bool GOMidiSystem::HasPendingDevices() const {
  const GOPortsConfig &portsConfig = m_config.GetMidiPortsConfig();

  return hasPendingDevices(
           m_config.m_MidiIn, m_midi_in_devices, portsConfig, m_MidiMap)
    || hasPendingDevices(
           m_config.m_MidiOut, m_midi_out_devices, portsConfig, m_MidiMap);
}

bool GOMidiSystem::hasPendingDevices(
  const GOMidiDeviceConfigList &configs,
  const ptr_vector<GOMidiPort> &ports,
  const GOPortsConfig &portsConfig,
  const GOMidiMap &midiMap) {
  return std::ranges::any_of(
    configs,
    [&ports, &portsConfig, &midiMap](const GOMidiDeviceConfig *config) {
      bool isPending = false;

      if (
        config->m_IsEnabled
        && portsConfig.IsEnabled(config->GetPortName(), config->GetApiName())) {
        const unsigned id
          = midiMap.GetDeviceIdByLogicalName(config->GetLogicalName());

        isPending = !std::ranges::any_of(ports, [id](const GOMidiPort *port) {
          return port->IsActive() && port->GetID() == id;
        });
      }
      return isPending;
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

    OpenDevices(false);
  }
}

void GOMidiSystem::OnDiscoveryTimer(wxTimerEvent &WXUNUSED(event)) {
  DiscoverDevices();
  if (!HasPendingDevices())
    EnsureDiscoveryTimerStopped();
}

void GOMidiSystem::OpenDevices(bool isToReconnectActive) {
  const GOPortsConfig &portsConfig(m_config.GetMidiPortsConfig());

  UpdateDevices(portsConfig);

  for (GOMidiPort *pPort : m_midi_in_devices)
    OpenOrClosePort(
      pPort,
      m_config.m_MidiIn,
      isToReconnectActive && m_config.IsToAutoAddMidi(),
      isToReconnectActive);
  for (GOMidiPort *pPort : m_midi_out_devices)
    OpenOrClosePort(pPort, m_config.m_MidiOut, false, isToReconnectActive);
}

void GOMidiSystem::OpenOrClosePort(
  GOMidiPort *pPort,
  GOMidiDeviceConfigList &configs,
  bool isToAutoAdd,
  bool isToReconnectActive) {
  const bool isToProcess = isToReconnectActive || !pPort->IsActive();
  const GOPortsConfig &portsConfig = m_config.GetMidiPortsConfig();

  if (isToProcess) {
    const wxString &portName = pPort->GetPortName();
    const wxString &apiName = pPort->GetApiName();
    GOMidiDeviceConfig *pDevConf = NULL;

    if (pPort->IsToUse() && portsConfig.IsEnabled(portName, apiName)) {
      const wxString &physicalName = pPort->GetName();

      pDevConf = configs.FindByPhysicalName(physicalName, portName, apiName);
      if (!pDevConf && isToAutoAdd)
        pDevConf = configs.Append(
          pPort->GetDefaultLogicalName(),
          pPort->GetDefaultRegEx(),
          portName,
          apiName,
          true,
          physicalName);
    }
    if (pDevConf && pDevConf->m_IsEnabled) {
      const bool isOpen = OpenPort(pPort, *pDevConf);

      // If opening raced device removal, allow the next scan to match
      // its replacement instead of retaining the failed port's physical
      // name.
      if (!isToReconnectActive && !isOpen)
        pDevConf->SetPhysicalName(wxEmptyString);
    } else if (pPort->IsActive())
      pPort->Close();
  }
}

bool GOMidiSystem::OpenPort(
  GOMidiPort *pPort, const GOMidiDeviceConfig &devConf) {
  const unsigned id = m_MidiMap.EnsureLogicalName(devConf.GetLogicalName());
  GOMidiInPort *pInPort = dynamic_cast<GOMidiInPort *>(pPort);
  bool isOpen = false;

  if (pInPort)
    isOpen = pInPort->Open(id, devConf.m_ChannelShift);
  else
    isOpen = pPort->Open(id);
  return isOpen;
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
