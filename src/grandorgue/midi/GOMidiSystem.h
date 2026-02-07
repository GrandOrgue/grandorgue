/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISYSTEM_H
#define GOMIDISYSTEM_H

#include <wx/event.h>

#include "config/GOPortsConfig.h"
#include "ports/GOMidiPortFactory.h"
#include "ptrvector.h"

class GOMidiEvent;
class GOMidiPort;
class GOMidiListener;
class GOMidiMap;
class GOConfig;
class GOOrganController;
class GOMidiWxEvent;

/**
 * This class represents a GrandOrgue-wide MIDI system. It may be used even
 * without any organ is loaded and without the Sound System is open
 */

class GOMidiSystem : public wxEvtHandler {
private:
  GOConfig &m_config;
  GOMidiMap &m_MidiMap;

  ptr_vector<GOMidiPort> m_midi_in_devices;
  ptr_vector<GOMidiPort> m_midi_out_devices;

  int m_transpose;
  std::vector<GOMidiListener *> m_Listeners;
  GOMidiPortFactory m_MidiFactory;

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
