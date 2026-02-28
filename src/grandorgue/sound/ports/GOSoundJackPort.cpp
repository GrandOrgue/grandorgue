/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

// wx should be included before windows.h (from jack.h), otherwise it cannot be
// compiled with mingw
#include "GOSoundJackPort.h"

#include <wx/log.h>

#include "config/GODeviceNamePattern.h"
#include "sound/buffer/GOSoundBufferMutableMono.h"

const wxString GOSoundJackPort::PORT_NAME = wxT("Jack");

GOSoundJackPort::GOSoundJackPort(GOSoundSystem *sound, wxString name)
  : GOSoundPort(sound, name) {}

GOSoundJackPort::~GOSoundJackPort() { Close(); }

#if defined(GO_USE_JACK)

#define MAX_CHANNELS_COUNT 64

static const jack_options_t JACK_OPTIONS = JackNullOption;
static const char *CLIENT_NAME = "GrandOrgueAudio";
static const wxString DEVICE_NAME = "Native Output";

void GOSoundJackPort::jackLatencyCallback(
  jack_latency_callback_mode_t mode, void *pData) {
  if (mode == JackPlaybackLatency) {
    GOSoundJackPort *const pPort = (GOSoundJackPort *)pData;

    if (pPort->m_Channels) {
      jack_latency_range_t range;

      jack_port_get_latency_range(
        pPort->mp_JackOutPorts[0], JackPlaybackLatency, &range);
      pPort->SetActualLatency(range.min / (double)pPort->m_SampleRate);
      wxLogDebug("JACK actual latency set to %d ms", pPort->m_ActualLatency);
    }
  }
}

int GOSoundJackPort::jackProcessCallback(jack_nframes_t nFrames, void *pData) {
  GOSoundJackPort *const pPort = (GOSoundJackPort *)pData;
  const bool isContinue = pPort->AudioCallback(pPort->m_GoBuffer);

  if (isContinue) {
    const unsigned nChannels = pPort->m_Channels;

    for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
      jack_default_audio_sample_t *pOut
        = (jack_default_audio_sample_t *)jack_port_get_buffer(
          pPort->mp_JackOutPorts[channelI], nFrames);
      GOSoundBufferMutableMono monoBuffer(pOut, nFrames);

      if (pPort->m_IsStarted)
        // copy samples from the interleaved pPort->m_GoBuffer to the non
        // interleaved jack buffer
        monoBuffer.CopyChannelFrom(pPort->m_GoBuffer, channelI);
      else
        // wipe the jack buffer
        monoBuffer.FillWithSilence();
    }
  }
  return isContinue ? 0 : 1;
}

void GOSoundJackPort::jackShutdownCallback(void *pData) {
  // GOSoundJackPort * const pPort = (GOSoundJackPort *) pData;
}

void GOSoundJackPort::Open() {
  Close();

  wxLogDebug("Connecting to a jack server");

  jack_status_t jack_status;

  mp_JackClient
    = jack_client_open(CLIENT_NAME, JACK_OPTIONS, &jack_status, nullptr);

  if (!mp_JackClient) {
    if (jack_status & JackServerFailed)
      throw wxString::Format("Unable to connect to a JACK server");
    throw wxString::Format(
      "jack_client_open() failed, status = 0x%2.0x", jack_status);
  }
  if (jack_status & JackServerStarted)
    wxLogDebug("JACK server started");
  if (jack_status & JackNameNotUnique)
    wxLogDebug(
      "Unique name `%s' assigned", jack_get_client_name(mp_JackClient));

  const jack_nframes_t sampleRate = jack_get_sample_rate(mp_JackClient);
  const jack_nframes_t samplesPerBuffer = jack_get_buffer_size(mp_JackClient);

  if (sampleRate != m_SampleRate)
    throw wxString::Format(
      "Device %s wants a different sample rate: %d.\nPlease adjust the "
      "GrandOrgue audio settings.",
      m_Name,
      sampleRate);
  if (samplesPerBuffer != m_SamplesPerBuffer)
    throw wxString::Format(
      "Device %s wants a different samples per buffer settings: %d.\nPlease "
      "adjust the GrandOrgue audio settings.",
      m_Name,
      samplesPerBuffer);

  char portName[32];

  if (m_Channels) {
    mp_JackOutPorts.clear();
    for (unsigned channelI = 0; channelI < m_Channels; ++channelI) {
      snprintf(portName, sizeof(portName), "out_%d", channelI);

      jack_port_t *pJackOutPort = jack_port_register(
        mp_JackClient, portName, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

      if (!pJackOutPort)
        throw wxString::Format("No more JACK ports available");
      mp_JackOutPorts.push_back(pJackOutPort);
    }
  }
  wxLogDebug("Created %d output ports", m_Channels);

  jack_set_latency_callback(mp_JackClient, &jackLatencyCallback, this);
  jack_set_process_callback(mp_JackClient, &jackProcessCallback, this);
  jack_on_shutdown(mp_JackClient, &jackShutdownCallback, this);

  m_GoBuffer.Resize(m_Channels, samplesPerBuffer);

  m_IsOpen = true;
}

void GOSoundJackPort::StartStream() {
  if (!mp_JackClient || !m_IsOpen)
    throw wxString::Format("Audio device %s not open", m_Name);
  m_IsStarted = true;
  if (jack_activate(mp_JackClient))
    wxString::Format("Cannot activate the jack client");
}

wxString GOSoundJackPort::getName() {
  return GOSoundPortFactory::getInstance().ComposeDeviceName(
    PORT_NAME, wxEmptyString, "Native Output");
}
#endif /* GO_USE_JACK */

void GOSoundJackPort::Close() {
#if defined(GO_USE_JACK)
  m_IsStarted = false;
  m_IsOpen = false;
  if (mp_JackClient) {
    jack_deactivate(mp_JackClient);
    wxLogDebug("Disconnecting from the jack server");
    jack_client_close(mp_JackClient);
    mp_JackClient = nullptr;
  }
  mp_JackOutPorts.clear();
#endif
}

static const wxString OLD_STYLE_NAME = wxT("Jack Output");

GOSoundPort *GOSoundJackPort::create(
  const GOPortsConfig &portsConfig,
  GOSoundSystem *sound,
  GODeviceNamePattern &pattern) {
  GOSoundPort *pPort = nullptr;
#if defined(GO_USE_JACK)
  const wxString devName = getName();

  if (
    portsConfig.IsEnabled(PORT_NAME)
    && (
      pattern.DoesMatch(devName)
      || pattern.DoesMatch(devName + GOPortFactory::c_NameDelim)
      || pattern.DoesMatch(OLD_STYLE_NAME))) {
    pattern.SetPhysicalName(devName);
    pPort = new GOSoundJackPort(sound, devName);
  }
#endif
  return pPort;
}

void GOSoundJackPort::addDevices(
  const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &result) {
#if defined(GO_USE_JACK)
  if (portsConfig.IsEnabled(PORT_NAME))
    result.emplace_back(
      PORT_NAME, wxEmptyString, DEVICE_NAME, MAX_CHANNELS_COUNT, false);
#endif
}
