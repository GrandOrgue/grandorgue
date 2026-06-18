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

#include "buffer/GOSoundBufferMutable.h"
#include "config/GOConfig.h"
#include "config/GOPortsConfig.h"
#include "ports/GOSoundPortFactory.h"
#include "threading/GOMultiMutexLocker.h"
#include "threading/GOMutexLocker.h"

#include "GOEvent.h"
#include "GOOrganController.h"
#include "GOSoundDefs.h"

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
    meter_counter(0),
    m_WaitCount(0),
    m_CalcCount(0) {}

GOSoundSystem::~GOSoundSystem() {
  AssureSoundIsClosed();

  GOMidiPortFactory::terminate();
  GOSoundPortFactory::terminate();
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
    // Callbacks fired during stream start are no-ops: the audio callback
    // checks m_IsRunning first and exits early while it is false.
    // m_IsRunning is set to true only in StartSoundSystem(), called after
    // OpenSoundSystem() completes.
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
  }
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

void GOSoundSystem::BuildAndStartEngine() {
  m_SoundEngine.BuildAndStart(
    m_config,
    m_SampleRate,
    m_SamplesPerBuffer,
    static_cast<GOOrganModel &>(*m_OrganController),
    m_OrganController->GetMemoryPool(),
    m_config.ReleaseConcurrency(),
    m_AudioRecorder);
}

void GOSoundSystem::StopAndDestroyEngine() { m_SoundEngine.StopAndDestroy(); }

bool GOSoundSystem::AssureSoundIsOpen() {
  if (!m_open) {
    OpenSoundSystem();
    if (m_open && m_OrganController) {
      BuildAndStartEngine();
      StartSoundSystem();
      NotifySoundIsOpen();
    }
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
  unsigned devIndex, GOSoundBufferMutable &outBuffer) {
  bool wasEntered = false;
  const unsigned nSamples = outBuffer.GetNFrames();

  if (m_IsRunning.load()) {
    if (nSamples == m_SamplesPerBuffer) {
      m_NCallbacksEntered.fetch_add(1);
      wasEntered = true;
    } else
      wxLogError(
        _("No sound output will happen. Samples per buffer has been "
          "changed by the sound driver to %d"),
        nSamples);
  }
  // assure that m_IsRunning has not yet been changed after
  // m_NCallbacksEntered.fetch_add, otherwise the control thread may not wait
  if (wasEntered && m_IsRunning.load()) {
    GOSoundOutput &device = m_AudioOutputs[devIndex];
    GOMutexLocker locker(device.mutex);

    while (device.wait && device.waiting)
      device.condition.Wait();

    unsigned cnt = m_CalcCount.fetch_add(1);
    m_SoundEngine.GetAudioOutput(
      devIndex, cnt + 1 >= m_AudioOutputs.size(), outBuffer);
    device.wait = true;
    unsigned count = m_WaitCount.fetch_add(1);

    if (count + 1 == m_AudioOutputs.size()) {
      m_SoundEngine.NextPeriod();
      UpdateMeter();

      m_SoundEngine.WakeupThreads();
      m_CalcCount.exchange(0);
      m_WaitCount.exchange(0);

      for (unsigned i = 0; i < m_AudioOutputs.size(); i++) {
        GOMutexLocker lock(m_AudioOutputs[i].mutex, i == devIndex);
        m_AudioOutputs[i].wait = false;
        m_AudioOutputs[i].condition.Signal();
      }
    }
  } else
    outBuffer.FillWithSilence();
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
