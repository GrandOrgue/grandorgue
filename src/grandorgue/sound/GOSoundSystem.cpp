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

#include "GOEvent.h"
#include "GOSoundDefs.h"
#include "GOSoundOrganEngine.h"

GOSoundSystem::GOSoundSystem(GOConfig &settings)
  : m_config(settings),
    p_CloseListener(nullptr),
    m_open(false),
    logSoundErrors(true),
    m_DefaultAudioDevice(GOSoundDevInfo::getInvalideDeviceInfo()),
    meter_counter(0) {}

GOSoundSystem::~GOSoundSystem() {
  AssureSoundIsClosed();

  GOSoundPortFactory::terminate();
}

void GOSoundSystem::OpenSoundSystem() {
  assert(!m_open);
  assert(mp_SoundPorts.empty());

  std::vector<GOAudioDeviceConfig> &audio_config
    = m_config.GetAudioDeviceConfig();

  m_LastErrorMessage = wxEmptyString;
  SetSampleRate(m_config.SampleRate());
  SetSamplesPerBuffer(m_config.SamplesPerBuffer());
  mp_SoundPorts.resize(audio_config.size());

  const GOPortsConfig &portsConfig(m_config.GetSoundPortsConfig());

  try {
    for (unsigned n = mp_SoundPorts.size(), i = 0; i < n; i++) {
      GOAudioDeviceConfig &deviceConfig = audio_config[i];
      GODeviceNamePattern *pNamePattern = &deviceConfig;
      GODeviceNamePattern defaultDevicePattern;

      if (!pNamePattern->IsFilled()) {
        FillDeviceNamePattern(
          GetDefaultAudioDevice(portsConfig), defaultDevicePattern);
        pNamePattern = &defaultDevicePattern;
      }

      GOSoundPort *pPort
        = GOSoundPortFactory::create(portsConfig, *this, *pNamePattern);

      if (!pPort)
        throw wxString::Format(
          _("Output device %s not found - no sound output will occur"),
          pNamePattern->GetRegEx());
      mp_SoundPorts[i].reset(pPort);
      pPort->Init(
        deviceConfig.GetChannels(),
        GetSampleRate(),
        GetSamplesPerBuffer(),
        deviceConfig.GetDesiredLatency(),
        i);
    }
    // Callbacks fired during stream start are no-ops: the audio callback
    // checks m_IsRunning first and exits early while it is false.
    // m_IsRunning is set to true only in StartSoundSystem(), called after
    // OpenSoundSystem() completes.
    StartStreams();
    m_open = true;
  } catch (wxString &msg) {
    if (logSoundErrors)
      GOMessageBox(msg, _("Error"), wxOK | wxICON_ERROR, NULL);
    else
      m_LastErrorMessage = msg;

    CloseSoundSystem();
  }
}

void GOSoundSystem::CloseSoundSystem() {
  for (int i = mp_SoundPorts.size() - 1; i >= 0; i--) {
    if (mp_SoundPorts[i]) {
      mp_SoundPorts[i]->Close();
      mp_SoundPorts[i].reset();
    }
  }

  ResetMeters();
  mp_SoundPorts.clear();
  m_open = false;
}

void GOSoundSystem::StartStreams() {
  for (auto &pPort : mp_SoundPorts)
    pPort->Open();

  if (GetSamplesPerBuffer() > MAX_FRAME_SIZE)
    throw wxString::Format(
      _("Cannot use buffer size above %d samples; "
        "unacceptable quantization would occur."),
      MAX_FRAME_SIZE);
  for (auto &pPort : mp_SoundPorts)
    pPort->StartStream();
}

bool GOSoundSystem::AssureSoundIsOpen() {
  if (!m_open) {
    OpenSoundSystem();
  }
  return m_open;
}

void GOSoundSystem::AssureSoundIsClosed() {
  if (m_open) {
    if (p_CloseListener) // The callback must call to DisconnectFromEngine()
      p_CloseListener->OnBeforeSoundClose();

    assert(!IsEngineConnected());

    CloseSoundSystem();
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
  meter_counter += GetSamplesPerBuffer();
  if (meter_counter >= 6144) // update 44100 / (N / 2) = ~14 times per second
  {
    wxCommandEvent event(wxEVT_METERS, 0);
    event.SetInt(0x0);
    if (wxTheApp->GetTopWindow())
      wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
    meter_counter = 0;
  }
}

wxString GOSoundSystem::getState() {
  if (!mp_SoundPorts.size())
    return _("No sound output occurring");
  wxString result = wxString::Format(
    _("%d samples per buffer, %d Hz\n"),
    GetSamplesPerBuffer(),
    GetSampleRate());

  for (auto &pPort : mp_SoundPorts)
    result = result + _("\n") + pPort->getPortState();
  return result;
}

void GOSoundSystem::OnBeforeConnectToEngine() {
  assert(m_open);
  assert(p_CloseListener);
}
