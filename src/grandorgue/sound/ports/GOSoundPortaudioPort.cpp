/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundPortaudioPort.h"

#include <wx/intl.h>
#include <wx/regex.h>

#include "config/GODeviceNamePattern.h"

const wxString GOSoundPortaudioPort::PORT_NAME = wxT("PortAudio");
const wxString GOSoundPortaudioPort::PORT_NAME_OLD = wxT("Pa");

wxString GOSoundPortaudioPort::getLastError(PaError error) {
  const char *errText = NULL;

  if (
    error == paUnanticipatedHostError
    || error
      == paInternalError // workaround of
                         // https://github.com/PortAudio/portaudio/issues/620
  ) {
    const PaHostErrorInfo *const pErrInfo = Pa_GetLastHostErrorInfo();

    if (
      pErrInfo != NULL && pErrInfo->errorText != NULL
      && strlen(pErrInfo->errorText))
      errText = pErrInfo->errorText;
  }
  if (errText == NULL)
    errText = Pa_GetErrorText(error);
  return wxGetTranslation(wxString::FromAscii(errText));
}

GOSoundPortaudioPort::GOSoundPortaudioPort(
  GOSound *sound, unsigned paDevIndex, const wxString &name)
  : GOSoundPort(sound, name), m_PaDevIndex(paDevIndex), m_stream(nullptr) {}

GOSoundPortaudioPort::~GOSoundPortaudioPort() { Close(); }

static wxString compose_device_name(
  const wxString &prefix, const PaDeviceInfo *pInfo) {
  const PaHostApiInfo *pApi = Pa_GetHostApiInfo(pInfo->hostApi);
  return GOSoundPortFactory::getFullDeviceName(
    prefix, wxString::FromAscii(pApi->name), wxString(pInfo->name));
}

static wxString compose_device_name(const PaDeviceInfo *pInfo) {
  return compose_device_name(GOSoundPortaudioPort::PORT_NAME, pInfo);
}

void GOSoundPortaudioPort::Open() {
  Close();

  PaStreamParameters stream_parameters;

  stream_parameters.device = m_PaDevIndex;
  stream_parameters.channelCount = m_Channels;
  stream_parameters.sampleFormat = paFloat32;
  stream_parameters.suggestedLatency = m_Latency / 1000.0;
  stream_parameters.hostApiSpecificStreamInfo = NULL;

  PaError error = Pa_OpenStream(
    &m_stream,
    NULL,
    &stream_parameters,
    m_SampleRate,
    m_SamplesPerBuffer,
    paNoFlag,
    &Callback,
    this);
  if (error != paNoError)
    throw wxString::Format(
      _("Open of the audio stream for %s failed: %s"),
      m_Name.c_str(),
      getLastError(error));
  m_IsOpen = true;
}

void GOSoundPortaudioPort::StartStream() {
  if (!m_stream || !m_IsOpen)
    throw wxString::Format(_("Audio device %s not open"), m_Name.c_str());

  PaError error;
  error = Pa_StartStream(m_stream);
  if (error != paNoError)
    throw wxString::Format(
      _("Start of audio stream of %s failed: %s"),
      m_Name.c_str(),
      getLastError(error));

  const struct PaStreamInfo *info = Pa_GetStreamInfo(m_stream);
  SetActualLatency(info->outputLatency);
}

void GOSoundPortaudioPort::Close() {
  if (!m_stream || !m_IsOpen)
    return;
  Pa_StopStream(m_stream);
  Pa_CloseStream(m_stream);
  m_stream = 0;
  m_IsOpen = false;
}

int GOSoundPortaudioPort::Callback(
  const void *input,
  void *output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo *timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData) {
  GOSoundPortaudioPort *port = (GOSoundPortaudioPort *)userData;
  if (port->AudioCallback((float *)output, frameCount))
    return paContinue;
  else
    return paAbort;
}

// for compatibility with old settings
wxString get_oldstyle_name(unsigned index) {
  const PaDeviceInfo *info = Pa_GetDeviceInfo(index);
  const PaHostApiInfo *api = Pa_GetHostApiInfo(info->hostApi);
  return wxGetTranslation(wxString::FromAscii(api->name))
    + wxString(_(" (PA): ")) + wxString(info->name);
}

static bool has_initialised = false;

void assure_initialised() {
  if (!has_initialised) {
    Pa_Initialize();
    has_initialised = true;
  }
}

void GOSoundPortaudioPort::terminate() {
  if (has_initialised) {
    Pa_Terminate();
    has_initialised = false;
  }
}

GOSoundPort *GOSoundPortaudioPort::create(
  const GOPortsConfig &portsConfig,
  GOSound *sound,
  GODeviceNamePattern &pattern) {
  GOSoundPort *pPort = nullptr;

  if (portsConfig.IsEnabled(PORT_NAME)) {
    assure_initialised();
    for (int i = 0; i < Pa_GetDeviceCount(); i++) {
      const PaDeviceInfo *pInfo = Pa_GetDeviceInfo(i);
      const wxString devName = compose_device_name(pInfo);

      if (
        pInfo->maxOutputChannels > 0
        && (
	  pattern.DoesMatch(devName)
	  || pattern.DoesMatch(devName + GOPortFactory::c_NameDelim)
	  || pattern.DoesMatch(get_oldstyle_name(i))
	  || pattern.DoesMatch(compose_device_name(PORT_NAME_OLD, pInfo)))) {
        pattern.SetPhysicalName(devName);
        pPort = new GOSoundPortaudioPort(sound, i, devName);
        break;
      }
    }
  }
  return pPort;
}

static const wxString WX_ALSA = wxT("ALSA");
static const wxRegEx REGEX_ALSA_NAME_PATTERN("(.+) \\(hw:([0-9]+),([0-9]+)\\)");

static void fill_logical_name_and_regex(GOSoundDevInfo &info) {
  const wxString &apiName = info.GetApiName();
  const wxString &deviceName = info.GetName();

  if (apiName == wxT("ALSA")) {
    if (REGEX_ALSA_NAME_PATTERN.Matches(deviceName)) {
      const wxString devName1 = REGEX_ALSA_NAME_PATTERN.GetMatch(deviceName, 1);
      const wxString devName3 = REGEX_ALSA_NAME_PATTERN.GetMatch(deviceName, 3);

      info.SetDefaultNameRegex(GOSoundPortFactory::getFullDeviceName(
        GOSoundPortaudioPort::PORT_NAME,
        apiName,
        wxString::Format("%s \\(hw:[0-9]+,%s\\)", devName1, devName3)));
      info.SetDefaultLogicalName(GOSoundPortFactory::getFullDeviceName(
        GOSoundPortaudioPort::PORT_NAME,
        apiName,
        wxString::Format("%s (hw:,%s)", devName1, devName3)));
    }
  }
}

void GOSoundPortaudioPort::addDevices(
  const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &result) {
  if (portsConfig.IsEnabled(PORT_NAME)) {
    assure_initialised();

    const PaDeviceIndex defaultIndex = Pa_GetDefaultOutputDevice();

    for (int i = 0; i < Pa_GetDeviceCount(); i++) {
      const PaDeviceInfo *pInfo = Pa_GetDeviceInfo(i);

      if (pInfo->maxOutputChannels > 0) {
        result.emplace_back(
          PORT_NAME,
          wxString::FromAscii(Pa_GetHostApiInfo(pInfo->hostApi)->name),
          wxString(pInfo->name),
          pInfo->maxOutputChannels,
          i == defaultIndex,
          fill_logical_name_and_regex);
      }
    }
  }
}
