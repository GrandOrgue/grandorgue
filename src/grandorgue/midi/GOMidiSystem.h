/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISYSTEM_H
#define GOMIDISYSTEM_H

#include <memory>

#include <wx/event.h>

#include "config/GOPortsConfig.h"
#include "ports/GOMidiPortFactory.h"
#include "ptrvector.h"

class GOMidiEvent;
class GOMidiPort;
class GOMidiListener;
class GOMidiMap;
class GOMidiDeviceConfig;
class GOMidiDeviceConfigList;
class GOConfig;
class GOOrganController;
class GOMidiWxEvent;
class wxTimer;
class wxTimerEvent;

/**
 * This class represents a GrandOrgue-wide MIDI system. It may be used even
 * without any organ is loaded and without the Sound System is open
 */

class GOMidiSystem : public wxEvtHandler {
  friend class GOTestMidiSystem;

private:
  GOConfig &m_config;
  GOMidiMap &m_MidiMap;

  ptr_vector<GOMidiPort> m_midi_in_devices;
  ptr_vector<GOMidiPort> m_midi_out_devices;

  int m_transpose;
  std::vector<GOMidiListener *> m_Listeners;
  GOMidiPortFactory m_MidiFactory;

  /**
   * Periodically discovers configured MIDI ports that were unavailable
   * when the MIDI system was opened
   */
  std::unique_ptr<wxTimer> o_DiscoveryTimer;

  /**
   * Stops the discovery timer if it is running
   */
  void EnsureDiscoveryTimerStopped();

  /**
   * Whether a configured and enabled MIDI port in configs has no
   * active port in ports
   */
  static bool hasPendingDevices(
    const GOMidiDeviceConfigList &configs,
    const ptr_vector<GOMidiPort> &ports,
    const GOPortsConfig &portsConfig,
    const GOMidiMap &midiMap);

  /**
   * Whether a configured and enabled MIDI input or output port has
   * no active port
   */
  bool HasPendingDevices() const;

  /**
   * Opens inactive configured ports so that a port appearing after
   * startup is used with its saved settings
   */
  void DiscoverDevices();

  /**
   * Timer handler; stops discovery once all configured ports are
   * active
   */
  void OnDiscoveryTimer(wxTimerEvent &event);

  /**
   * Opens or closes the ports of one direction according to their configs
   *
   * @param isToReconnectActive whether already active ports are reopened.
   * Discovery passes false, so that an active port is not reopened even if
   * the backend now reports a different native address for it; an explicit
   * Open() passes true, to apply the settings and reset the MIDI state
   */
  void OpenDevices(bool isToReconnectActive);

  /**
   * Opens or closes one port according to its device config
   *
   * @param pPort the port to process
   * @param configs the device configs of the direction of pPort
   * @param isToAutoAdd whether a config is created for a port which has none
   * @param isToReconnectActive whether an already active port is reopened
   */
  void OpenOrClosePort(
    GOMidiPort *pPort,
    GOMidiDeviceConfigList &configs,
    bool isToAutoAdd,
    bool isToReconnectActive);

  /**
   * Opens one port with the settings of its device config
   *
   * @param pPort the port to open
   * @param devConf the config holding the logical name and the channel shift
   * @return whether the port is active after the attempt
   */
  bool OpenPort(GOMidiPort *pPort, const GOMidiDeviceConfig &devConf);

public:
  GOMidiSystem(GOConfig &settings);
  ~GOMidiSystem();

  void Open();
  void UpdateDevices(const GOPortsConfig &portsConfig);

  void Recv(const GOMidiEvent &e);
  void PlayEvent(const GOMidiEvent &e);
  void OnMidiEvent(GOMidiWxEvent &e);
  void Send(const GOMidiEvent &e);

  const ptr_vector<GOMidiPort> &GetInDevices() const {
    return m_midi_in_devices;
  }
  const ptr_vector<GOMidiPort> &GetOutDevices() const {
    return m_midi_out_devices;
  }
  bool HasActiveDevice();
  int GetTranspose();
  void SetTranspose(int transpose);
  void Register(GOMidiListener *listener);
  void Unregister(GOMidiListener *listener);

  GOMidiMap &GetMidiMap() { return m_MidiMap; }

  DECLARE_EVENT_TABLE()
};

#endif /* GOMIDISYSTEM_H */
