/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSYSTEM_H
#define GOSOUNDSYSTEM_H

#include <map>
#include <vector>

#include <wx/string.h>

#include "config/GOPortsConfig.h"
#include "midi/GOMidiSystem.h"
#include "ports/GOSoundPortFactory.h"
#include "threading/GOCondition.h"
#include "threading/GOMutex.h"

#include "ptrvector.h"

#include "GOSoundDevInfo.h"
#include "GOSoundOrganEngine.h"
#include "GOSoundRecorder.h"

class GODeviceNamePattern;
class GOOrganController;
class GOMidiSystem;
class GOSoundThread;
class GOSoundPort;
class GOSoundRtPort;
class GOSoundPortaudioPort;
class GOConfig;

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
  bool m_open;
  std::atomic_bool m_IsRunning;

  // counter of audio callbacks that have been entered but have not yet been
  // exited
  std::atomic_uint m_NCallbacksEntered;

  // For waiting for and notifying when m_NCallbacksEntered bacomes 0
  GOMutex m_CallbackMutex;
  GOCondition m_CallbackCondition;

  GOMutex m_lock;
  GOMutex m_thread_lock;

  bool logSoundErrors;

  std::vector<GOSoundOutput> m_AudioOutputs;
  std::atomic_uint m_WaitCount;
  std::atomic_uint m_CalcCount;

  unsigned m_SamplesPerBuffer;

  unsigned meter_counter;

  GOSoundDevInfo m_DefaultAudioDevice;

  GOOrganController *m_OrganController;
  GOSoundRecorder m_AudioRecorder;

  GOSoundOrganEngine m_SoundEngine;
  ptr_vector<GOSoundThread> m_Threads;

  GOConfig &m_config;

  GOMidiSystem m_midi;

  wxString m_LastErrorMessage;

  void StopThreads();
  void StartThreads();

  void ResetMeters();

  void OpenMidi();

  void OpenSound();
  void CloseSound();

  void StartStreams();
  void UpdateMeter();

public:
  GOSoundSystem(GOConfig &settings);
  ~GOSoundSystem();

  bool AssureSoundIsOpen();
  void AssureSoundIsClosed();

  wxString getLastErrorMessage() const { return m_LastErrorMessage; }
  wxString getState();

  GOConfig &GetSettings();

  void AssignOrganFile(GOOrganController *organController);
  GOOrganController *GetOrganFile();

  void SetLogSoundErrorMessages(bool settingsDialogVisible);

  std::vector<GOSoundDevInfo> GetAudioDevices(const GOPortsConfig &portsConfig);
  const GOSoundDevInfo &GetDefaultAudioDevice(const GOPortsConfig &portsConfig);

  static void FillDeviceNamePattern(
    const GOSoundDevInfo &deviceInfo, GODeviceNamePattern &pattern);

  GOMidiSystem &GetMidi();

  GOSoundOrganEngine &GetEngine();

  bool AudioCallback(
    unsigned dev_index, float *outputBuffer, unsigned int nFrames);
};

#endif
