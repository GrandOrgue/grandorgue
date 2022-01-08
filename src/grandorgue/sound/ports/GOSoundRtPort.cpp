/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundRtPort.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GOSoundPortFactory.h"

const wxString GOSoundRtPort::PORT_NAME = wxT("Rt");

GOSoundRtPort::GOSoundRtPort(GOSound *sound, RtAudio *rtApi, wxString name)
    : GOSoundPort(sound, name), m_rtApi(rtApi), m_nBuffers(0) {}

GOSoundRtPort::~GOSoundRtPort() {
  Close();
  try {
    if (m_rtApi) {
      const RtAudio *rtApi = m_rtApi;

      m_rtApi = NULL;
      delete rtApi;
    }
  } catch (RtAudioError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    wxLogError(_("RtAudio error: %s"), error.c_str());
  }
}

void GOSoundRtPort::Open() {
  Close();
  if (!m_rtApi)
    throw wxString::Format(
      _("Audio device %s not initialised"), m_Name.c_str());

  try {
    RtAudio::StreamParameters aOutputParam;
    aOutputParam.deviceId = -1;
    aOutputParam.nChannels = m_Channels;

    for (unsigned i = 0; i < m_rtApi->getDeviceCount(); i++)
      if (getName(m_rtApi, i) == m_Name) aOutputParam.deviceId = i;

    RtAudio::StreamOptions aOptions;
    // the next flag causes Rt/Core forces setting the buffer size to 15
    // and the sound distortion https://github.com/oleg68/GrandOrgue/issues/54
    // aOptions.flags = RTAUDIO_MINIMIZE_LATENCY;
    aOptions.numberOfBuffers
      = (m_Latency * m_SampleRate) / (m_SamplesPerBuffer * 1000);
    aOptions.streamName = "GrandOrgue";

    unsigned samples_per_buffer = m_SamplesPerBuffer;

    m_rtApi->openStream(
      &aOutputParam,
      NULL,
      RTAUDIO_FLOAT32,
      m_SampleRate,
      &samples_per_buffer,
      &Callback,
      this,
      &aOptions);
    m_nBuffers = aOptions.numberOfBuffers;
    if (samples_per_buffer != m_SamplesPerBuffer) {
      if (samples_per_buffer != m_SamplesPerBuffer)
        throw wxString::Format(
          _("Device %s wants a different samples per buffer settings: "
            "%d.\nPlease adjust the GrandOrgue audio settings."),
          m_Name.c_str(),
          samples_per_buffer);
    }
    m_IsOpen = true;
  } catch (RtAudioError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    throw wxString::Format(_("RtAudio error: %s"), error.c_str());
  }
}

void GOSoundRtPort::StartStream() {
  if (!m_rtApi || !m_IsOpen)
    throw wxString::Format(_("Audio device %s not open"), m_Name.c_str());

  try {
    m_rtApi->startStream();
    double actual_latency = m_rtApi->getStreamLatency();

    /* getStreamLatency returns zero if not supported by the API, in which
     * case we will make a best guess.
     */
    if (actual_latency == 0) actual_latency = m_SamplesPerBuffer * m_nBuffers;

    SetActualLatency(actual_latency / m_SampleRate);
  } catch (RtAudioError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    throw wxString::Format(_("RtAudio error: %s"), error.c_str());
  }

  if (m_rtApi->getStreamSampleRate() != m_SampleRate)
    throw wxString::Format(
      _("Sample rate of device %s changed"), m_Name.c_str());
}

void GOSoundRtPort::Close() {
  if (!m_rtApi || !m_IsOpen) return;
  try {
    m_rtApi->abortStream();
  } catch (RtAudioError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    wxLogError(_("RtAudio error: %s"), error.c_str());
  }
  try {
    m_rtApi->closeStream();
  } catch (RtAudioError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    wxLogError(_("RtAudio error: %s"), error.c_str());
  }
  m_IsOpen = false;
}

int GOSoundRtPort::Callback(
  void *outputBuffer,
  void *inputBuffer,
  unsigned int nFrames,
  double streamTime,
  RtAudioStreamStatus status,
  void *userData) {
  GOSoundRtPort *port = (GOSoundRtPort *)userData;
  if (port->AudioCallback((float *)outputBuffer, nFrames))
    return 0;
  else
    return 1;
}

wxString GOSoundRtPort::getName(RtAudio *rt_api, unsigned index) {
  wxString apiName = RtAudio::getApiName(rt_api->getCurrentApi());
  wxString devName;

  try {
    RtAudio::DeviceInfo info = rt_api->getDeviceInfo(index);
    devName = wxString(info.name);
  } catch (RtAudioError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    wxLogError(_("RtAudio error: %s"), error.c_str());
    devName = wxString::Format(_("<unknown> %d"), index);
  }
  return GOSoundPortFactory::getInstance().ComposeDeviceName(
    PORT_NAME, apiName, devName);
}

wxString get_oldstyle_name(RtAudio::Api api, RtAudio *rt_api, unsigned index) {
  wxString apiName;

  switch (api) {
    case RtAudio::LINUX_ALSA:
      apiName = wxT("Alsa");
      break;
    case RtAudio::LINUX_OSS:
      apiName = wxT("OSS");
      break;
    case RtAudio::LINUX_PULSE:
      apiName = wxT("PulseAudio");
      break;
    case RtAudio::MACOSX_CORE:
      apiName = wxT("Core");
      break;
    case RtAudio::UNIX_JACK:
      apiName = wxT("Jack");
      break;
    case RtAudio::WINDOWS_ASIO:
      apiName = wxT("ASIO");
      break;
    case RtAudio::WINDOWS_DS:
      apiName = wxT("DirectSound");
      break;
    case RtAudio::WINDOWS_WASAPI:
      apiName = wxT("WASAPI");
      break;
    case RtAudio::UNSPECIFIED:
    default:
      apiName = wxT("Unknown");
  }

  wxString prefix = apiName + wxT(": ");
  wxString devName;
  try {
    RtAudio::DeviceInfo info = rt_api->getDeviceInfo(index);
    devName = wxString(info.name);
  } catch (RtAudioError &e) {
    wxString error = wxString::FromAscii(e.getMessage().c_str());
    wxLogError(_("RtAudio error: %s"), error.c_str());
    devName = wxString::Format(_("<unknown> %d"), index);
  }
  return apiName + wxT(": ") + devName;
}

static bool hasApiNamesPopulated = false;
static std::vector<wxString> apiNames;

const std::vector<wxString> &GOSoundRtPort::getApis() {
  if (!hasApiNamesPopulated) {
    std::vector<RtAudio::Api> apiIndices;
    RtAudio::getCompiledApi(apiIndices);

    for (unsigned k = 0; k < apiIndices.size(); k++) {
      apiNames.push_back(wxString(RtAudio::getApiName(apiIndices[k])));
    }
    hasApiNamesPopulated = true;
  }
  return apiNames;
}

GOSoundPort *GOSoundRtPort::create(
  const GOPortsConfig &portsConfig, GOSound *sound, wxString name) {
  GOSoundRtPort *port = NULL;

  if (portsConfig.IsEnabled(PORT_NAME)) try {
      GOSoundPortFactory::NameParser parser(name);
      const wxString subsysName = parser.nextComp();
      wxString apiName = subsysName == PORT_NAME ? parser.nextComp() : wxT("");

      std::vector<RtAudio::Api> rtaudio_apis;
      RtAudio::getCompiledApi(rtaudio_apis);

      for (unsigned k = 0; k < rtaudio_apis.size(); k++) {
        const RtAudio::Api apiIndex = rtaudio_apis[k];
        const wxString rtApiName = RtAudio::getApiName(apiIndex);

        if (
          portsConfig.IsEnabled(PORT_NAME, rtApiName)
          && (apiName == rtApiName || apiName.IsEmpty())) {
          RtAudio *rtApi = NULL;

          try {
            rtApi = new RtAudio(apiIndex);
            unsigned int deviceCount = rtApi->getDeviceCount();

            for (unsigned i = 0; i < deviceCount; i++) {
              const wxString devName = getName(rtApi, i);

              if (
                devName == name || devName + GOPortFactory::c_NameDelim == name
                || (apiName.IsEmpty() && get_oldstyle_name(apiIndex, rtApi, i) == name)) {
                port = new GOSoundRtPort(sound, rtApi, devName);
                break;
              }
            }
          } catch (RtAudioError &e) {
            wxString error = wxString::FromAscii(e.getMessage().c_str());
            wxLogError(_("RtAudio error: %s"), error.c_str());
          }
          if (port) break;
          // here audioApi is not used by port
          if (rtApi) delete rtApi;
        }
      }
    } catch (RtAudioError &e) {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtAudio error: %s"), error.c_str());
    }
  return port;
}

void GOSoundRtPort::addDevices(
  const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &result) {
  if (portsConfig.IsEnabled(PORT_NAME)) try {
      std::vector<RtAudio::Api> rtaudio_apis;
      RtAudio::getCompiledApi(rtaudio_apis);

      for (unsigned k = 0; k < rtaudio_apis.size(); k++) {
        const RtAudio::Api apiIndex = rtaudio_apis[k];

        if (portsConfig.IsEnabled(PORT_NAME, RtAudio::getApiName(apiIndex))) {
          RtAudio *rtApi = nullptr;

          try {
            rtApi = new RtAudio(apiIndex);
            for (unsigned i = 0; i < rtApi->getDeviceCount(); i++) {
              RtAudio::DeviceInfo dev_info = rtApi->getDeviceInfo(i);
              if (!dev_info.probed) continue;
              if (dev_info.outputChannels < 1) continue;
              GOSoundDevInfo info;
              info.channels = dev_info.outputChannels;
              info.isDefault = dev_info.isDefaultOutput;
              info.name = getName(rtApi, i);
              result.push_back(info);
            }
          } catch (RtAudioError &e) {
            wxString error = wxString::FromAscii(e.getMessage().c_str());
            wxLogError(_("RtAudio error: %s"), error.c_str());
          }
          if (rtApi) delete rtApi;
        }
      }
    } catch (RtAudioError &e) {
      wxString error = wxString::FromAscii(e.getMessage().c_str());
      wxLogError(_("RtAudio error: %s"), error.c_str());
    }
}
