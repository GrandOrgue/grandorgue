/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSYSTEM_H
#define GOSOUNDSYSTEM_H

#include <atomic>
#include <memory>
#include <vector>

#include <wx/string.h>

#include "midi/GOMidiSystem.h"
#include "threading/GOCondition.h"
#include "threading/GOMutex.h"

#include "ptrvector.h"

#include "GOSoundCloseListener.h"
#include "GOSoundDevInfo.h"
#include "GOSoundOrganEngine.h"
#include "GOSoundRecorder.h"

class GOConfig;
class GODeviceNamePattern;
class GOOrganController;
class GOPortsConfig;
class GOSoundBufferMutable;
class GOSoundPort;

/**
 * This class represents a GrandOrgue-wide sound system. It may be used even
 * without a loaded organ
 */

class GOSoundSystem {
  class GOSoundOutput {
  public:
    GOSoundPort *port;
    GOMutex mutex;
    GOCondition condition;
    bool wait;
    bool waiting;

    GOSoundOutput() : condition(mutex) {
      port = 0;
      wait = false;
      waiting = false;
    }

    GOSoundOutput(const GOSoundOutput &old) : condition(mutex) {
      port = old.port;
      wait = old.wait;
      waiting = old.waiting;
    }

    const GOSoundOutput &operator=(const GOSoundOutput &old) {
      port = old.port;
      wait = old.wait;
      waiting = old.waiting;
      return *this;
    }
  };

private:
  GOConfig &m_config;

  GOMidiSystem m_midi;
  GOSoundRecorder m_AudioRecorder;
  std::unique_ptr<GOSoundOrganEngine> mp_SoundEngine;

  GOSoundCloseListener *p_CloseListener;

  bool m_open;
  bool logSoundErrors;
  unsigned m_SampleRate;
  unsigned m_SamplesPerBuffer;
  std::vector<GOSoundOutput> m_AudioOutputs;

  wxString m_LastErrorMessage;

  GOOrganController *m_OrganController;

  GOSoundDevInfo m_DefaultAudioDevice;

  // counter of audio callbacks that have been entered but have not yet been
  // exited
  std::atomic_uint m_NCallbacksEntered;

  // For waiting for and notifying when m_NCallbacksEntered bacomes 0
  GOMutex m_CallbackMutex;
  GOCondition m_CallbackCondition;

  GOMutex m_lock;

  unsigned meter_counter;

  std::atomic_uint m_WaitCount;
  std::atomic_uint m_CalcCount;

  void StartStreams();
  void OpenMidi() { m_midi.Open(); }

  void UpdateMeter();
  void ResetMeters();

  /** Open audio ports and configure the sound engine (without organ setup) */
  void OpenSoundSystem();
  /** Close and delete audio ports, reset meters, mark system as closed */
  void CloseSoundSystem();

public:
  static void FillDeviceNamePattern(
    const GOSoundDevInfo &deviceInfo, GODeviceNamePattern &pattern);

  GOSoundSystem(GOConfig &settings);
  ~GOSoundSystem();

  GOConfig &GetSettings() { return m_config; }
  GOMidiSystem &GetMidi() { return m_midi; }
  GOSoundOrganEngine &GetEngine() { return *mp_SoundEngine; }

  std::vector<GOSoundDevInfo> GetAudioDevices(const GOPortsConfig &portsConfig);
  const GOSoundDevInfo &GetDefaultAudioDevice(const GOPortsConfig &portsConfig);
  wxString getLastErrorMessage() const { return m_LastErrorMessage; }
  GOOrganController *GetOrganFile() { return m_OrganController; }

  /** Returns true if the sound system is currently open (audio ports active).
   */
  bool IsOpen() const { return m_open; }

  /** Returns the audio recorder associated with this sound system. */
  GOSoundRecorder &GetAudioRecorder() { return m_AudioRecorder; }

  unsigned GetSampleRate() const { return m_SampleRate; }
  unsigned GetSamplesPerBuffer() const { return m_SamplesPerBuffer; }
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
  void AssignOrganFile(GOOrganController *pNewOrganController);

  bool AudioCallback(unsigned devIndex, GOSoundBufferMutable &outBuffer);

  void ConnectToEngine(GOSoundOrganEngine &engine);
  void DisconnectFromEngine(GOSoundOrganEngine &engine);
};

#endif
