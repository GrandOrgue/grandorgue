/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundSystem.h"

#include <wx/app.h>
#include <wx/intl.h>
#include <wx/window.h>

#include "GOEvent.h"
#include "GOOrganController.h"
#include "GOSoundDefs.h"
#include "config/GOConfig.h"
#include "config/GOPortsConfig.h"
#include "ports/GOSoundPortFactory.h"
#include "scheduler/GOSoundThread.h"
#include "sound/ports/GOSoundPort.h"
#include "threading/GOMultiMutexLocker.h"
#include "threading/GOMutexLocker.h"

GOSoundSystem::GOSoundSystem(GOConfig &settings)
  : m_config(settings),
    m_midi(settings),
    m_open(false),
    logSoundErrors(true),
    m_SampleRate(0),
    m_SamplesPerBuffer(0),
    m_OrganController(0),
    m_DefaultAudioDevice(GOSoundDevInfo::getInvalideDeviceInfo()),
    m_IsRunning(false),
    m_NCallbacksEntered(0),
    m_CallbackCondition(m_CallbackMutex),
    meter_counter(0) {}

GOSoundSystem::~GOSoundSystem() {
  AssureSoundIsClosed();

  GOMidiPortFactory::terminate();
  GOSoundPortFactory::terminate();
}

void GOSoundSystem::StartThreads() {
  StopThreads();

  unsigned n_cpus = m_config.Concurrency();

  GOMutexLocker thread_locker(m_thread_lock);
  for (unsigned i = 0; i < n_cpus; i++)
    m_Threads.push_back(new GOSoundThread(&GetEngine().GetScheduler()));

  for (unsigned i = 0; i < m_Threads.size(); i++)
    m_Threads[i]->Run();
}

void GOSoundSystem::StopThreads() {
  for (unsigned i = 0; i < m_Threads.size(); i++)
    m_Threads[i]->Delete();

  GOMutexLocker thread_locker(m_thread_lock);
  m_Threads.resize(0);
}

void GOSoundSystem::OpenSoundSystem() {
  assert(!m_open);
  assert(m_AudioOutputs.size() == 0);

  std::vector<GOAudioDeviceConfig> &audio_config
    = m_config.GetAudioDeviceConfig();

  m_LastErrorMessage = wxEmptyString;
  m_SampleRate = m_config.SampleRate();
  m_SamplesPerBuffer = m_config.SamplesPerBuffer();
  m_AudioRecorder.SetBytesPerSample(m_config.WaveFormatBytesPerSample());

  m_AudioOutputs.resize(audio_config.size());
  for (GOSoundOutput &output : m_AudioOutputs)
    output.port = NULL;

  const GOPortsConfig &portsConfig(m_config.GetSoundPortsConfig());

  try {
    for (unsigned n = m_AudioOutputs.size(), i = 0; i < n; i++) {
      GOAudioDeviceConfig &deviceConfig = audio_config[i];
      GODeviceNamePattern *pNamePattern = &deviceConfig;
      GODeviceNamePattern defaultDevicePattern;

      if (!pNamePattern->IsFilled()) {
        FillDeviceNamePattern(
          GetDefaultAudioDevice(portsConfig), defaultDevicePattern);
        pNamePattern = &defaultDevicePattern;
      }

      GOSoundPort *pPort
        = GOSoundPortFactory::create(portsConfig, this, *pNamePattern);

      if (!pPort)
        throw wxString::Format(
          _("Output device %s not found - no sound output will occur"),
          pNamePattern->GetRegEx());
      m_AudioOutputs[i].port = pPort;
      pPort->Init(
        deviceConfig.GetChannels(),
        m_SampleRate,
        m_SamplesPerBuffer,
        deviceConfig.GetDesiredLatency(),
        i);
    }
    StartStreams();
    OpenMidi();
    m_AudioRecorder.SetSampleRate(m_SampleRate);
    m_open = true;
  } catch (wxString &msg) {
    if (logSoundErrors)
      GOMessageBox(msg, _("Error"), wxOK | wxICON_ERROR, NULL);
    else
      m_LastErrorMessage = msg;

    CloseSoundSystem();
  }
}

void GOSoundSystem::BuildAndStartEngine() {
  const unsigned audio_group_count = m_config.GetAudioGroups().size();
  std::vector<GOAudioDeviceConfig> &audio_config
    = m_config.GetAudioDeviceConfig();
  const unsigned audioDeviceCount = audio_config.size();
  std::vector<GOAudioOutputConfiguration> engine_config;

  engine_config.resize(audioDeviceCount);
  for (unsigned deviceI = 0; deviceI < audioDeviceCount; deviceI++) {
    const GOAudioDeviceConfig &deviceConfig = audio_config[deviceI];
    const auto &deviceOutputs = deviceConfig.GetChannelOututs();
    GOAudioOutputConfiguration &engineConfig = engine_config[deviceI];

    engineConfig.channels = deviceConfig.GetChannels();
    engineConfig.scale_factors.resize(engineConfig.channels);
    for (unsigned channelI = 0; channelI < engineConfig.channels; channelI++) {
      std::vector<float> &scaleFactors = engineConfig.scale_factors[channelI];

      scaleFactors.resize(audio_group_count * 2);
      std::fill(
        scaleFactors.begin(),
        scaleFactors.end(),
        GOAudioDeviceConfig::MUTE_VOLUME);

      if (channelI < deviceOutputs.size()) {
        for (const auto &groupOutput : deviceOutputs[channelI]) {
          int id = m_config.GetStrictAudioGroupId(groupOutput.GetName());

          if (id >= 0) {
            scaleFactors[id * 2] = groupOutput.GetLeft();
            scaleFactors[id * 2 + 1] = groupOutput.GetRight();
          }
        }
      }
    }
  }

  m_SoundEngine.SetSamplesPerBuffer(m_SamplesPerBuffer);
  m_SoundEngine.SetPolyphonyLimiting(m_config.ManagePolyphony());
  m_SoundEngine.SetHardPolyphony(m_config.PolyphonyLimit());
  m_SoundEngine.SetScaledReleases(m_config.ScaleRelease());
  m_SoundEngine.SetRandomizeSpeaking(m_config.RandomizeSpeaking());
  m_SoundEngine.SetInterpolationType(m_config.m_InterpolationType());
  m_SoundEngine.SetAudioGroupCount(audio_group_count);
  m_SoundEngine.SetSampleRate(m_SampleRate);
  m_SoundEngine.SetAudioOutput(engine_config);
  m_SoundEngine.SetupReverb(m_config);
  m_SoundEngine.SetAudioRecorder(&m_AudioRecorder, m_config.RecordDownmix());
  m_SoundEngine.Setup(
    *m_OrganController,
    m_OrganController->GetMemoryPool(),
    m_config.ReleaseConcurrency());
  StartThreads();
  m_SoundEngine.GetScheduler().ResumeGivingWork();
}

void GOSoundSystem::StartSoundSystem() {
  // Enable all outputs
  for (GOSoundOutput &output : m_AudioOutputs) {
    GOMutexLocker dev_lock(output.mutex);

    output.wait = false;
    output.waiting = true;
  }
  m_WaitCount.store(0);
  m_CalcCount.store(0);
  m_NCallbacksEntered.store(0);
  m_IsRunning.store(true);
}

void GOSoundSystem::NotifySoundIsOpen() {
  m_OrganController->PreparePlayback(
    &GetEngine(), &GetMidi(), &m_AudioRecorder);
}

void GOSoundSystem::NotifySoundIsClosing() { m_OrganController->Abort(); }

void GOSoundSystem::StopSoundSystem() {
  m_IsRunning.store(false);

  // wait for all started callbacks to finish
  {
    GOMutexLocker lock(m_CallbackMutex);

    while (m_NCallbacksEntered.load() > 0)
      m_CallbackCondition.WaitOrStop(
        "GOSoundSystem::StopSoundSystem waits for all callbacks to finish",
        nullptr);
  }

  // Disable all outputs
  {
    GOMultiMutexLocker multi;

    for (GOSoundOutput &output : m_AudioOutputs)
      multi.Add(output.mutex);

    for (GOSoundOutput &output : m_AudioOutputs) {
      output.waiting = false;
      output.wait = false;
      output.condition.Broadcast();
    }

    for (unsigned n = m_AudioOutputs.size(), i = 1; i < n; i++) {
      GOMutexLocker dev_lock(m_AudioOutputs[i].mutex);
      m_AudioOutputs[i].condition.Broadcast();
    }
  }
}

void GOSoundSystem::StopAndDestroyEngine() {
  m_SoundEngine.GetScheduler().PauseGivingWork();
  for (GOSoundThread *pThread : m_Threads)
    pThread->WaitForIdle();
  StopThreads();
  m_SoundEngine.ClearSetup();
}

void GOSoundSystem::CloseSoundSystem() {
  for (int i = m_AudioOutputs.size() - 1; i >= 0; i--) {
    if (m_AudioOutputs[i].port) {
      GOSoundPort *const port = m_AudioOutputs[i].port;

      m_AudioOutputs[i].port = NULL;
      port->Close();
      delete port;
    }
  }

  ResetMeters();
  m_AudioOutputs.clear();
  m_open = false;
}

void GOSoundSystem::StartStreams() {
  for (GOSoundOutput &output : m_AudioOutputs)
    output.port->Open();

  if (m_SamplesPerBuffer > MAX_FRAME_SIZE)
    throw wxString::Format(
      _("Cannot use buffer size above %d samples; "
        "unacceptable quantization would occur."),
      MAX_FRAME_SIZE);
  for (GOSoundOutput &output : m_AudioOutputs)
    output.port->StartStream();
}

bool GOSoundSystem::AssureSoundIsOpen() {
  if (!m_open)
    OpenSoundSystem();
  if (m_open && m_OrganController) {
    BuildAndStartEngine();
    StartSoundSystem();
    NotifySoundIsOpen();
  }
  return m_open;
}

void GOSoundSystem::AssureSoundIsClosed() {
  if (m_open) {
    if (m_OrganController) {
      NotifySoundIsClosing();
      StopSoundSystem();
      StopAndDestroyEngine();
    }
    CloseSoundSystem();
  }
}

void GOSoundSystem::AssignOrganFile(GOOrganController *pNewOrganController) {
  if (pNewOrganController != m_OrganController) {
    GOMutexLocker locker(m_lock);

    if (m_open && m_OrganController) {
      NotifySoundIsClosing();
      StopSoundSystem();
      StopAndDestroyEngine();
    }

    m_OrganController = pNewOrganController;

    if (m_open && m_OrganController) {
      BuildAndStartEngine();
      StartSoundSystem();
      NotifySoundIsOpen();
    }
  }
}

std::vector<GOSoundDevInfo> GOSoundSystem::GetAudioDevices(
  const GOPortsConfig &portsConfig) {
  // Getting a device list tries to open and close each device
  // Because some devices (ex. ASIO) cann't be open more than once
  // then close the current audio device
  AssureSoundIsClosed();
  m_DefaultAudioDevice = GOSoundDevInfo::getInvalideDeviceInfo();

  std::vector<GOSoundDevInfo> list
    = GOSoundPortFactory::getDeviceList(portsConfig);

  for (const auto &devInfo : list)
    if (devInfo.IsDefault()) {
      m_DefaultAudioDevice = devInfo;
      break;
    }
  return list;
}

const GOSoundDevInfo &GOSoundSystem::GetDefaultAudioDevice(
  const GOPortsConfig &portsConfig) {
  if (!m_DefaultAudioDevice.IsValid())
    GetAudioDevices(portsConfig);
  return m_DefaultAudioDevice;
}

void GOSoundSystem::FillDeviceNamePattern(
  const GOSoundDevInfo &deviceInfo, GODeviceNamePattern &pattern) {
  pattern.SetLogicalName(deviceInfo.GetDefaultLogicalName());
  pattern.SetRegEx(deviceInfo.GetDefaultNameRegex());
  pattern.SetPortName(deviceInfo.GetPortName());
  pattern.SetApiName(deviceInfo.GetApiName());
  pattern.SetPhysicalName(deviceInfo.GetFullName());
}

void GOSoundSystem::ResetMeters() {
  wxWindow *const topWindow = wxTheApp ? wxTheApp->GetTopWindow() : nullptr;

  if (topWindow) {
    wxCommandEvent event(wxEVT_METERS, 0);

    event.SetInt(0x1);
    topWindow->GetEventHandler()->AddPendingEvent(event);
  }
}

void GOSoundSystem::UpdateMeter() {
  /* Update meters */
  meter_counter += m_SamplesPerBuffer;
  if (meter_counter >= 6144) // update 44100 / (N / 2) = ~14 times per second
  {
    wxCommandEvent event(wxEVT_METERS, 0);
    event.SetInt(0x0);
    if (wxTheApp->GetTopWindow())
      wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
    meter_counter = 0;
  }
}

bool GOSoundSystem::AudioCallback(
  unsigned dev_index, float *output_buffer, unsigned int n_frames) {
  bool wasEntered = false;

  if (m_IsRunning.load()) {
    if (n_frames == m_SamplesPerBuffer) {
      m_NCallbacksEntered.fetch_add(1);
      wasEntered = true;
    } else
      wxLogError(
        _("No sound output will happen. Samples per buffer has been "
          "changed by the sound driver to %d"),
        n_frames);
  }
  // assure that m_IsRunning has not yet been changed after
  // m_NCallbacksEntered.fetch_add, otherwise the control thread may not wait
  if (wasEntered && m_IsRunning.load()) {
    GOSoundOutput *device = &m_AudioOutputs[dev_index];
    GOMutexLocker locker(device->mutex);

    while (device->wait && device->waiting)
      device->condition.Wait();

    unsigned cnt = m_CalcCount.fetch_add(1);
    m_SoundEngine.GetAudioOutput(
      output_buffer, n_frames, dev_index, cnt + 1 >= m_AudioOutputs.size());
    device->wait = true;
    unsigned count = m_WaitCount.fetch_add(1);

    if (count + 1 == m_AudioOutputs.size()) {
      m_SoundEngine.NextPeriod();
      UpdateMeter();

      {
        GOMutexLocker thread_locker(m_thread_lock);
        for (unsigned i = 0; i < m_Threads.size(); i++)
          m_Threads[i]->Wakeup();
      }
      m_CalcCount.exchange(0);
      m_WaitCount.exchange(0);

      for (unsigned i = 0; i < m_AudioOutputs.size(); i++) {
        GOMutexLocker lock(m_AudioOutputs[i].mutex, i == dev_index);
        m_AudioOutputs[i].wait = false;
        m_AudioOutputs[i].condition.Signal();
      }
    }
  } else
    m_SoundEngine.GetEmptyAudioOutput(dev_index, n_frames, output_buffer);
  if (
    wasEntered && m_NCallbacksEntered.fetch_sub(1) <= 1
    && !m_IsRunning.load()) {
    // ensure that the control thread enters into m_NCallbackCondition.Wait()
    GOMutexLocker lk(m_CallbackMutex);

    // notify the control thread
    m_CallbackCondition.Broadcast();
  }
  return true;
}

wxString GOSoundSystem::getState() {
  if (!m_AudioOutputs.size())
    return _("No sound output occurring");
  wxString result = wxString::Format(
    _("%d samples per buffer, %d Hz\n"),
    m_SamplesPerBuffer,
    m_SoundEngine.GetSampleRate());
  for (unsigned i = 0; i < m_AudioOutputs.size(); i++)
    result = result + _("\n") + m_AudioOutputs[i].port->getPortState();
  return result;
}
