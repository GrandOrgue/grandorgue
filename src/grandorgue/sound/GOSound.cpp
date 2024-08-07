/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSound.h"

#include <wx/app.h>
#include <wx/intl.h>
#include <wx/window.h>

#include "GOEvent.h"
#include "GOOrganController.h"
#include "GOSoundDefs.h"
#include "config/GOConfig.h"
#include "midi/GOMidi.h"
#include "scheduler/GOSoundThread.h"
#include "sound/ports/GOSoundPort.h"
#include "threading/GOMultiMutexLocker.h"
#include "threading/GOMutexLocker.h"

GOSound::GOSound(GOConfig &settings)
  : m_open(false),
    m_IsRunning(false),
    m_NCallbacksEntered(0),
    m_CallbackCondition(m_CallbackMutex),
    logSoundErrors(true),
    m_AudioOutputs(),
    m_WaitCount(),
    m_CalcCount(),
    m_SamplesPerBuffer(0),
    meter_counter(0),
    m_DefaultAudioDevice(GOSoundDevInfo::getInvalideDeviceInfo()),
    m_OrganController(0),
    m_config(settings),
    m_midi(settings) {}

GOSound::~GOSound() {
  CloseSound();

  GOMidiPortFactory::terminate();
  GOSoundPortFactory::terminate();
}

void GOSound::StartThreads() {
  StopThreads();

  unsigned n_cpus = m_config.Concurrency();

  GOMutexLocker thread_locker(m_thread_lock);
  for (unsigned i = 0; i < n_cpus; i++)
    m_Threads.push_back(new GOSoundThread(&GetEngine().GetScheduler()));

  for (unsigned i = 0; i < m_Threads.size(); i++)
    m_Threads[i]->Run();
}

void GOSound::StopThreads() {
  for (unsigned i = 0; i < m_Threads.size(); i++)
    m_Threads[i]->Delete();

  GOMutexLocker thread_locker(m_thread_lock);
  m_Threads.resize(0);
}

void GOSound::OpenMidi() { m_midi.Open(); }

void GOSound::OpenSound() {
  m_LastErrorMessage = wxEmptyString;
  assert(!m_open);
  assert(m_AudioOutputs.size() == 0);

  const unsigned audio_group_count = m_config.GetAudioGroups().size();
  std::vector<GOAudioDeviceConfig> &audio_config
    = m_config.GetAudioDeviceConfig();
  const unsigned audioDeviceCount = audio_config.size();
  std::vector<GOAudioOutputConfiguration> engine_config;

  m_AudioOutputs.resize(audio_config.size());
  for (unsigned i = 0; i < m_AudioOutputs.size(); i++)
    m_AudioOutputs[i].port = NULL;
  engine_config.resize(audioDeviceCount);
  for (unsigned i = 0; i < audioDeviceCount; i++) {
    const GOAudioDeviceConfig &deviceConfig = audio_config[i];
    const auto &deviceOutputs = deviceConfig.GetChannelOututs();
    GOAudioOutputConfiguration &engineConfig = engine_config[i];

    engineConfig.channels = deviceConfig.GetChannels();
    engineConfig.scale_factors.resize(engineConfig.channels);
    for (unsigned j = 0; j < engineConfig.channels; j++) {
      std::vector<float> &scaleFactors = engineConfig.scale_factors[j];

      scaleFactors.resize(audio_group_count * 2);
      std::fill(
        scaleFactors.begin(),
        scaleFactors.end(),
        GOAudioDeviceConfig::MUTE_VOLUME);

      if (j >= deviceOutputs.size())
        continue;

      const auto &channelOutputs = deviceOutputs[j];

      for (unsigned k = 0; k < channelOutputs.size(); k++) {
        const auto &groupOutput = channelOutputs[k];
        int id = m_config.GetStrictAudioGroupId(groupOutput.GetName());

        if (id >= 0) {
          scaleFactors[id * 2] = groupOutput.GetLeft();
          scaleFactors[id * 2 + 1] = groupOutput.GetRight();
        }
      }
    }
  }
  m_SamplesPerBuffer = m_config.SamplesPerBuffer();
  m_SoundEngine.SetSamplesPerBuffer(m_SamplesPerBuffer);
  m_SoundEngine.SetPolyphonyLimiting(m_config.ManagePolyphony());
  m_SoundEngine.SetHardPolyphony(m_config.PolyphonyLimit());
  m_SoundEngine.SetScaledReleases(m_config.ScaleRelease());
  m_SoundEngine.SetRandomizeSpeaking(m_config.RandomizeSpeaking());
  m_SoundEngine.SetInterpolationType(m_config.InterpolationType());
  m_SoundEngine.SetAudioGroupCount(audio_group_count);
  unsigned sample_rate = m_config.SampleRate();
  m_AudioRecorder.SetBytesPerSample(m_config.WaveFormatBytesPerSample());
  GetEngine().SetSampleRate(sample_rate);
  m_AudioRecorder.SetSampleRate(sample_rate);
  m_SoundEngine.SetAudioOutput(engine_config);
  m_SoundEngine.SetupReverb(m_config);
  m_SoundEngine.SetAudioRecorder(&m_AudioRecorder, m_config.RecordDownmix());

  if (m_OrganController)
    m_SoundEngine.Setup(m_OrganController, m_config.ReleaseConcurrency());
  else
    m_SoundEngine.ClearSetup();

  const GOPortsConfig &portsConfig(m_config.GetSoundPortsConfig());

  try {
    for (unsigned l = m_AudioOutputs.size(), i = 0; i < l; i++) {
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
        GetEngine().GetSampleRate(),
        m_SamplesPerBuffer,
        deviceConfig.GetDesiredLatency(),
        i);
    }

    OpenMidi();
    m_NCallbacksEntered.store(0);
    StartStreams();
    StartThreads();
    m_open = true;
    m_IsRunning.store(true);

    if (m_OrganController)
      m_OrganController->PreparePlayback(
        &GetEngine(), &GetMidi(), &m_AudioRecorder);
  } catch (wxString &msg) {
    if (logSoundErrors)
      GOMessageBox(msg, _("Error"), wxOK | wxICON_ERROR, NULL);
    else
      m_LastErrorMessage = msg;
  }

  if (!m_open)
    CloseSound();
}

void GOSound::StartStreams() {
  for (unsigned i = 0; i < m_AudioOutputs.size(); i++)
    m_AudioOutputs[i].port->Open();

  if (m_SamplesPerBuffer > MAX_FRAME_SIZE)
    throw wxString::Format(
      _("Cannot use buffer size above %d samples; "
        "unacceptable quantization would occur."),
      MAX_FRAME_SIZE);

  m_WaitCount.exchange(0);
  m_CalcCount.exchange(0);
  for (unsigned i = 0; i < m_AudioOutputs.size(); i++) {
    GOMutexLocker dev_lock(m_AudioOutputs[i].mutex);
    m_AudioOutputs[i].wait = false;
    m_AudioOutputs[i].waiting = true;
  }

  for (unsigned i = 0; i < m_AudioOutputs.size(); i++)
    m_AudioOutputs[i].port->StartStream();
}

void GOSound::CloseSound() {
  m_IsRunning.store(false);

  // wait for all started callbacks to finish
  {
    GOMutexLocker lock(m_CallbackMutex);

    while (m_NCallbacksEntered.load() > 0)
      m_CallbackCondition.WaitOrStop(
        "GOSound::CloseSound waits for all callbacks to finish", nullptr);
  }

  StopThreads();

  for (unsigned i = 0; i < m_AudioOutputs.size(); i++) {
    m_AudioOutputs[i].waiting = false;
    m_AudioOutputs[i].wait = false;
    m_AudioOutputs[i].condition.Broadcast();
  }

  for (unsigned i = 1; i < m_AudioOutputs.size(); i++) {
    GOMutexLocker dev_lock(m_AudioOutputs[i].mutex);
    m_AudioOutputs[i].condition.Broadcast();
  }

  for (int i = m_AudioOutputs.size() - 1; i >= 0; i--) {
    if (m_AudioOutputs[i].port) {
      GOSoundPort *const port = m_AudioOutputs[i].port;

      m_AudioOutputs[i].port = NULL;
      port->Close();
      delete port;
    }
  }

  if (m_OrganController)
    m_OrganController->Abort();
  ResetMeters();
  m_AudioOutputs.clear();
  m_open = false;
}

bool GOSound::AssureSoundIsOpen() {
  if (!m_open)
    OpenSound();
  return m_open;
}

void GOSound::AssureSoundIsClosed() {
  if (m_open)
    CloseSound();
}

void GOSound::AssignOrganFile(GOOrganController *organController) {
  if (organController == m_OrganController)
    return;

  GOMutexLocker locker(m_lock);
  GOMultiMutexLocker multi;
  for (unsigned i = 0; i < m_AudioOutputs.size(); i++)
    multi.Add(m_AudioOutputs[i].mutex);

  if (m_OrganController) {
    // ensure pointers to work items are not held by threads
    m_SoundEngine.GetScheduler().PauseGivingWork();
    for (GOSoundThread *thread : m_Threads)
      thread->WaitForIdle();

    m_OrganController->Abort();
    // now work items are safe to be deleted
    m_SoundEngine.ClearSetup();

    // resume processing of work items
    m_SoundEngine.GetScheduler().ResumeGivingWork();
  }

  m_OrganController = organController;

  if (m_OrganController && m_AudioOutputs.size()) {
    m_SoundEngine.Setup(organController, m_config.ReleaseConcurrency());
    m_OrganController->PreparePlayback(
      &GetEngine(), &GetMidi(), &m_AudioRecorder);
  }
}

GOConfig &GOSound::GetSettings() { return m_config; }

GOOrganController *GOSound::GetOrganFile() { return m_OrganController; }

void GOSound::SetLogSoundErrorMessages(bool settingsDialogVisible) {
  logSoundErrors = settingsDialogVisible;
}

std::vector<GOSoundDevInfo> GOSound::GetAudioDevices(
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

const GOSoundDevInfo &GOSound::GetDefaultAudioDevice(
  const GOPortsConfig &portsConfig) {
  if (!m_DefaultAudioDevice.IsValid())
    GetAudioDevices(portsConfig);
  return m_DefaultAudioDevice;
}

void GOSound::FillDeviceNamePattern(
  const GOSoundDevInfo &deviceInfo, GODeviceNamePattern &pattern) {
  pattern.SetLogicalName(deviceInfo.GetDefaultLogicalName());
  pattern.SetRegEx(deviceInfo.GetDefaultNameRegex());
  pattern.SetPortName(deviceInfo.GetPortName());
  pattern.SetApiName(deviceInfo.GetApiName());
  pattern.SetPhysicalName(deviceInfo.GetFullName());
}

GOMidi &GOSound::GetMidi() { return m_midi; }

void GOSound::ResetMeters() {
  wxCommandEvent event(wxEVT_METERS, 0);
  event.SetInt(0x1);
  if (wxTheApp->GetTopWindow())
    wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

void GOSound::UpdateMeter() {
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

bool GOSound::AudioCallback(
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

GOSoundEngine &GOSound::GetEngine() { return m_SoundEngine; }

wxString GOSound::getState() {
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
