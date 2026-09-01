/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSYSTEM_H
#define GOSOUNDSYSTEM_H

#include <memory>
#include <vector>

#include <wx/string.h>

#include "interfaces/GOSoundCallbackConnector.h"
#include "interfaces/GOSoundCloseListener.h"
#include "threading/GOMutex.h"

#include "GOSoundDevInfo.h"

class GOConfig;
class GODeviceNamePattern;
class GOPortsConfig;
class GOSoundPort;

/**
 * This class represents a GrandOrgue-wide sound system. It may be used even
 * without a loaded organ
 */

class GOSoundSystem : public GOSoundCallbackConnector {
private:
  GOConfig &m_config;

  GOSoundCloseListener *p_CloseListener;

  bool m_open;
  bool logSoundErrors;
  std::vector<std::unique_ptr<GOSoundPort>> mp_SoundPorts;

  wxString m_LastErrorMessage;

  GOSoundDevInfo m_DefaultAudioDevice;

  GOMutex m_lock;

  unsigned meter_counter;

  void StartStreams();

  void UpdateMeter();
  void ResetMeters();

  /** Open audio ports and configure the sound engine (without organ setup) */
  void OpenSoundSystem();
  /** Close and delete audio ports, reset meters, mark system as closed */
  void CloseSoundSystem();

protected:
  void OnBeforeConnectToEngine() override;
  void OnNewAudioPeriod() override { UpdateMeter(); }

public:
  static void FillDeviceNamePattern(
    const GOSoundDevInfo &deviceInfo, GODeviceNamePattern &pattern);

  GOSoundSystem(GOConfig &settings);
  ~GOSoundSystem();

  std::vector<GOSoundDevInfo> GetAudioDevices(const GOPortsConfig &portsConfig);
  const GOSoundDevInfo &GetDefaultAudioDevice(const GOPortsConfig &portsConfig);
  wxString getLastErrorMessage() const { return m_LastErrorMessage; }

  /** Returns true if the sound system is currently open (audio ports active).
   */
  bool IsOpen() const { return m_open; }

  wxString getState();

  void SetLogSoundErrorMessages(bool isVisible) { logSoundErrors = isVisible; }

  /**
   * Sets the listener to be notified before the sound system closes its audio
   * ports. Pass nullptr to unregister. The caller must ensure the listener
   * outlives this sound system (or unregisters before being destroyed).
   */
  void SetCloseListener(GOSoundCloseListener *pListener) {
    p_CloseListener = pListener;
  }

  bool AssureSoundIsOpen();
  void AssureSoundIsClosed();
};

#endif
