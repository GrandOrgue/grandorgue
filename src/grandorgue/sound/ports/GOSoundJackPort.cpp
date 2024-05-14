/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

// wx should be included before windows.h (from jack.h), otherwise it cannot be
// compiled with mingw
#include "GOSoundJackPort.h"

#include <wx/log.h>

const wxString GOSoundJackPort::PORT_NAME = wxT("Jack");

GOSoundJackPort::GOSoundJackPort(GOSound *sound, wxString name)
  : GOSoundPort(sound, name) {}

GOSoundJackPort::~GOSoundJackPort() { Close(); }

#if defined(GO_USE_JACK)

#define MAX_CHANNELS_COUNT 64

static const jack_options_t JACK_OPTIONS = JackNullOption;
static const char *CLIENT_NAME = "GrandOrgueAudio";

void GOSoundJackPort::JackLatencyCallback(
  jack_latency_callback_mode_t mode, void *data) {
  if (mode == JackPlaybackLatency) {
    GOSoundJackPort *const jp = (GOSoundJackPort *)data;

    if (jp->m_Channels) {
      jack_latency_range_t range;

      jack_port_get_latency_range(
        jp->m_JackOutputPorts[0], JackPlaybackLatency, &range);
      jp->SetActualLatency(range.min / (double)jp->m_SampleRate);
      wxLogDebug("JACK actual latency set to %d ms", jp->m_ActualLatency);
    }
  }
}

int GOSoundJackPort::JackProcessCallback(jack_nframes_t nFrames, void *data) {
  int rc = 0;
  GOSoundJackPort *const port = (GOSoundJackPort *)data;

  if (port->AudioCallback(port->m_GoBuffer, nFrames)) {
    const unsigned int nc = port->m_Channels;

    for (unsigned int i = 0; i < nc; i++) {
      jack_default_audio_sample_t *out
        = (jack_default_audio_sample_t *)jack_port_get_buffer(
          port->m_JackOutputPorts[i], nFrames);

      if (port->m_IsStarted) {
        // copy samples from the interleaved port->m_GoBuffer to the non
        // interleaved jack buffer
        float *in = port->m_GoBuffer + i;

        for (unsigned int j = 0; j < nFrames; j++) {
          *(out++) = *in;
          in += nc;
        }
      } else {
        // wipe the jack buffer
        memset(out, 0, sizeof(jack_default_audio_sample_t) * nFrames);
      }
    }
  } else
    rc = 1;
  return rc;
}

void GOSoundJackPort::JackShutdownCallback(void *data) {
  // GOSoundJackPort * const jp = (GOSoundJackPort *) data;
}

void GOSoundJackPort::Open() {
  Close();

  wxLogDebug("Connecting to a jack server");

  jack_status_t jack_status;

  m_JackClient
    = jack_client_open(CLIENT_NAME, JACK_OPTIONS, &jack_status, NULL);

  if (!m_JackClient) {
    if (jack_status & JackServerFailed)
      throw wxString::Format("Unable to connect to a JACK server");
    throw wxString::Format(
      "jack_client_open() failed, status = 0x%2.0x", jack_status);
  }
  if (jack_status & JackServerStarted)
    wxLogDebug("JACK server started");
  if (jack_status & JackNameNotUnique)
    wxLogDebug("Unique name `%s' assigned", jack_get_client_name(m_JackClient));

  const jack_nframes_t sample_rate = jack_get_sample_rate(m_JackClient);
  const jack_nframes_t samples_per_buffer = jack_get_buffer_size(m_JackClient);

  if (sample_rate != m_SampleRate)
    throw wxString::Format(
      "Device %s wants a different sample rate: %d.\nPlease adjust the "
      "GrandOrgue audio settings.",
      m_Name,
      sample_rate);
  if (samples_per_buffer != m_SamplesPerBuffer)
    throw wxString::Format(
      "Device %s wants a different samples per buffer settings: %d.\nPlease "
      "adjust the GrandOrgue audio settings.",
      m_Name,
      samples_per_buffer);

  char port_name[32];

  if (m_Channels) {
    m_JackOutputPorts = new jack_port_t *[m_Channels];
    for (unsigned int i = 0; i < m_Channels; i++) {
      sprintf(port_name, "out_%d", i);
      m_JackOutputPorts[i] = jack_port_register(
        m_JackClient, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
      if (m_JackOutputPorts[i] == NULL)
        throw wxString::Format("No more JACK ports available");
    }
  }
  wxLogDebug("Created %d output ports", m_Channels);

  jack_set_latency_callback(m_JackClient, &JackLatencyCallback, this);
  jack_set_process_callback(m_JackClient, &JackProcessCallback, this);
  jack_on_shutdown(m_JackClient, &JackShutdownCallback, this);

  m_GoBuffer = new float[samples_per_buffer * m_Channels];

  m_IsOpen = true;
}

void GOSoundJackPort::StartStream() {
  if (!m_JackClient || !m_IsOpen)
    throw wxString::Format("Audio device %s not open", m_Name);
  m_IsStarted = true;
  if (jack_activate(m_JackClient))
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
  if (m_JackClient) {
    jack_deactivate(m_JackClient);
    wxLogDebug("Disconnecting from the jack server");
    jack_client_close(m_JackClient);
    m_JackClient = NULL;
  }
  if (m_JackOutputPorts) {
    delete m_JackOutputPorts;
    m_JackOutputPorts = NULL;
  }
  if (m_GoBuffer) {
    delete m_GoBuffer;
    m_GoBuffer = NULL;
  }
#endif
}

static const wxString OLD_STYLE_NAME = wxT("Jack Output");

GOSoundPort *GOSoundJackPort::create(
  const GOPortsConfig &portsConfig, GOSound *sound, wxString name) {
#if defined(GO_USE_JACK)
  const wxString devName = getName();

  if (
    portsConfig.IsEnabled(PORT_NAME)
    && (name == devName || devName + GOPortFactory::c_NameDelim == name || name == OLD_STYLE_NAME))
    return new GOSoundJackPort(sound, devName);
#endif
  return NULL;
}

void GOSoundJackPort::addDevices(
  const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &result) {
#if defined(GO_USE_JACK)
  if (portsConfig.IsEnabled(PORT_NAME)) {
    GOSoundDevInfo info;

    info.channels = MAX_CHANNELS_COUNT;
    info.isDefault = true;
    info.name = getName();
    result.push_back(info);
  }
#endif
}
