/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundRtPort.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GOSoundPortFactory.h"

const wxString GOSoundRtPort::PORT_NAME = wxT("RtAudio");
const wxString GOSoundRtPort::PORT_NAME_OLD = wxT("Rt");

GOSoundRtPort::GOSoundRtPort(
  GOSound *sound, RtAudio *rtApi, unsigned rtDevId, const wxString &name)
  : GOSoundPort(sound, name),
    m_rtApi(rtApi),
    m_RtDevId(rtDevId),
    m_nBuffers(0) {}

GOSoundRtPort::~GOSoundRtPort() {
  Close();
  if (m_rtApi) {
    const RtAudio *rtApi = m_rtApi;

    m_rtApi = NULL;
    delete rtApi;
  }
}

bool GOSoundRtPort::processRtResult(
  RtAudioErrorType rtResult, bool isToThrowOnError) {
  bool isOk = true;

  if (rtResult > RTAUDIO_NO_ERROR) {
    wxString rtMsg = wxString::Format(
      rtResult > RTAUDIO_WARNING ? _("RtAudio error: %s")
                                 : _("RtAudio warning: %s"),
      m_rtApi->getErrorText());

    if (rtResult > RTAUDIO_WARNING) {
      if (isToThrowOnError)
        throw rtMsg;
      wxLogError(rtMsg);
    } else
      wxLogWarning(rtMsg);
  }
  return isOk;
}

void GOSoundRtPort::Open() {
  Close();
  if (!m_rtApi)
    throw wxString::Format(
      _("Audio device %s not initialised"), m_Name.c_str());

  RtAudio::StreamParameters aOutputParam;
  RtAudio::StreamOptions aOptions;

  aOutputParam.deviceId = m_RtDevId;
  aOutputParam.nChannels = m_Channels;
  // the next flag causes Rt/Core forces setting the buffer size to 15
  // and the sound distortion https://github.com/oleg68/GrandOrgue/issues/54
  // aOptions.flags = RTAUDIO_MINIMIZE_LATENCY;
  aOptions.numberOfBuffers
    = (m_Latency * m_SampleRate) / (m_SamplesPerBuffer * 1000);
  aOptions.streamName = "GrandOrgue";

  unsigned samples_per_buffer = m_SamplesPerBuffer;

  processRtResult(m_rtApi->openStream(
    &aOutputParam,
    NULL,
    RTAUDIO_FLOAT32,
    m_SampleRate,
    &samples_per_buffer,
    &Callback,
    this,
    &aOptions));
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
}

void GOSoundRtPort::StartStream() {
  if (!m_rtApi || !m_IsOpen)
    throw wxString::Format(_("Audio device %s not open"), m_Name.c_str());

  processRtResult(m_rtApi->startStream());

  double actual_latency = m_rtApi->getStreamLatency();

  /* getStreamLatency returns zero if not supported by the API, in which
   * case we will make a best guess.
   */
  if (actual_latency == 0)
    actual_latency = m_SamplesPerBuffer * m_nBuffers;

  SetActualLatency(actual_latency / m_SampleRate);

  if (m_rtApi->getStreamSampleRate() != m_SampleRate)
    throw wxString::Format(
      _("Sample rate of device %s changed"), m_Name.c_str());
}

void GOSoundRtPort::Close() {
  if (!m_rtApi || !m_IsOpen)
    return;
  processRtResult(m_rtApi->abortStream(), false);
  m_rtApi->closeStream();
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

wxString compose_device_name(
  const wxString &prefix, RtAudio *rt_api, const RtAudio::DeviceInfo &info) {
  wxString apiName = RtAudio::getApiName(rt_api->getCurrentApi());

  return GOSoundPortFactory::getInstance().ComposeDeviceName(
    prefix, apiName, wxString(info.name));
}

wxString GOSoundRtPort::getName(
  RtAudio *rtApi, const RtAudio::DeviceInfo &devInfo) {
  return compose_device_name(PORT_NAME, rtApi, devInfo);
}

wxString get_oldstyle_name(
  RtAudio::Api api, RtAudio *rt_api, const RtAudio::DeviceInfo &info) {
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
  return apiName + wxT(": ") + wxString(info.name);
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

  if (portsConfig.IsEnabled(PORT_NAME)) {
    GOSoundPortFactory::NameParser parser(name);
    const wxString subsysName = parser.nextComp();
    wxString apiName = (subsysName == PORT_NAME || subsysName == PORT_NAME_OLD)
      ? parser.nextComp()
      : wxT("");

    std::vector<RtAudio::Api> rtaudio_apis;
    RtAudio::getCompiledApi(rtaudio_apis);

    for (unsigned k = 0; k < rtaudio_apis.size(); k++) {
      const RtAudio::Api apiIndex = rtaudio_apis[k];
      const wxString rtApiName = RtAudio::getApiName(apiIndex);

      if (
        portsConfig.IsEnabled(PORT_NAME, rtApiName)
        && (apiName == rtApiName || apiName.IsEmpty())) {
        RtAudio *rtApi = NULL;

        rtApi = new RtAudio(apiIndex);
        for (auto deviceId : rtApi->getDeviceIds()) {
          const RtAudio::DeviceInfo info = rtApi->getDeviceInfo(deviceId);
          const wxString devName = getName(rtApi, info);

          if (
            (devName == name || devName + GOPortFactory::c_NameDelim == name
             || (apiName.IsEmpty() && get_oldstyle_name(apiIndex, rtApi, info) == name)
             || compose_device_name(PORT_NAME_OLD, rtApi, info) == name)
            && // skip input-only devices that may have the same name (osx
               // usb)
            info.outputChannels > 0) {
            port = new GOSoundRtPort(sound, rtApi, deviceId, devName);
            break;
          }
        }
        if (port)
          break;
        // here audioApi is not used by port
        if (rtApi)
          delete rtApi;
      }
    }
  }
  return port;
}

void GOSoundRtPort::addDevices(
  const GOPortsConfig &portsConfig, std::vector<GOSoundDevInfo> &result) {
  if (portsConfig.IsEnabled(PORT_NAME)) {
    std::vector<RtAudio::Api> rtaudio_apis;
    RtAudio::getCompiledApi(rtaudio_apis);

    for (unsigned k = 0; k < rtaudio_apis.size(); k++) {
      const RtAudio::Api apiIndex = rtaudio_apis[k];

      if (portsConfig.IsEnabled(PORT_NAME, RtAudio::getApiName(apiIndex))) {
        RtAudio *rtApi = nullptr;

        rtApi = new RtAudio(apiIndex);
        for (auto deviceId : rtApi->getDeviceIds()) {
          RtAudio::DeviceInfo dev_info = rtApi->getDeviceInfo(deviceId);

          if (dev_info.ID && dev_info.outputChannels > 0) {
            GOSoundDevInfo info;

            info.channels = dev_info.outputChannels;
            info.isDefault = dev_info.isDefaultOutput;
            info.name = getName(rtApi, dev_info);
            result.push_back(info);
          }
        }
        if (rtApi)
          delete rtApi;
      }
    }
  }
}
