/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSoundPortaudioPort.h"

#include <wx/intl.h>

const wxString GOrgueSoundPortaudioPort::PORT_NAME = wxT("Pa");

wxString GOrgueSoundPortaudioPort::getLastError(PaError error)
{
  const char* errText = NULL;
  
  if (
    error == paUnanticipatedHostError
    || error == paInternalError // workaround of https://github.com/PortAudio/portaudio/issues/620
  ) {
    const PaHostErrorInfo * const pErrInfo = Pa_GetLastHostErrorInfo();
    
    if (pErrInfo != NULL && pErrInfo->errorText != NULL && strlen(pErrInfo->errorText))
      errText = pErrInfo->errorText;
  }
  if (errText == NULL)
    errText = Pa_GetErrorText(error);
  return wxGetTranslation(wxString::FromAscii(errText));
}

GOrgueSoundPortaudioPort::GOrgueSoundPortaudioPort(GOrgueSound* sound, wxString name) :
	GOrgueSoundPort(sound, name),
	m_stream(0)
{
}

GOrgueSoundPortaudioPort::~GOrgueSoundPortaudioPort()
{
	Close();
}

wxString GOrgueSoundPortaudioPort::getName(unsigned index)
{
	const PaDeviceInfo* info = Pa_GetDeviceInfo(index);
	const PaHostApiInfo *api = Pa_GetHostApiInfo(info->hostApi);
	return composeDeviceName(PORT_NAME, wxString::FromAscii(api->name), wxString(info->name));
}

void GOrgueSoundPortaudioPort::Open()
{
	Close();
		
	PaStreamParameters stream_parameters;
	stream_parameters.device = -1;
	stream_parameters.channelCount = m_Channels;
	stream_parameters.sampleFormat = paFloat32;
	stream_parameters.suggestedLatency = m_Latency / 1000.0;
	stream_parameters.hostApiSpecificStreamInfo = NULL;

	for(int i = 0; i < Pa_GetDeviceCount(); i++)
		if (getName(i) == m_Name)
			stream_parameters.device = i;

	PaError error;
	error = Pa_OpenStream(&m_stream, NULL, &stream_parameters, m_SampleRate, m_SamplesPerBuffer, paNoFlag, &Callback, this);
	if (error != paNoError)
		throw wxString::Format(_("Open of the audio stream for %s failed: %s"), m_Name.c_str(), getLastError(error));
	m_IsOpen = true;
}

void GOrgueSoundPortaudioPort::StartStream()
{
	if (!m_stream || !m_IsOpen)
		throw wxString::Format(_("Audio device %s not open"), m_Name.c_str());

	PaError error;
	error = Pa_StartStream(m_stream);
	if (error != paNoError)
		throw wxString::Format(_("Start of audio stream of %s failed: %s"), m_Name.c_str(),
				       getLastError(error));

	const struct PaStreamInfo* info = Pa_GetStreamInfo(m_stream);
	SetActualLatency(info->outputLatency);
}

void GOrgueSoundPortaudioPort::Close()
{
	if (!m_stream || !m_IsOpen)
		return;
	Pa_StopStream(m_stream);
	Pa_CloseStream(m_stream);
	m_stream = 0;
	m_IsOpen = false;
}

int GOrgueSoundPortaudioPort::Callback (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	GOrgueSoundPortaudioPort* port = (GOrgueSoundPortaudioPort*)userData;
	if (port->AudioCallback((float*)output, frameCount))
		return paContinue;
	else
		return paAbort;
}

// for compatibility with old settings
wxString get_oldstyle_name(unsigned index)
{
	const PaDeviceInfo* info = Pa_GetDeviceInfo(index);
	const PaHostApiInfo *api = Pa_GetHostApiInfo(info->hostApi);
	return wxGetTranslation(wxString::FromAscii(api->name)) + wxString(_(" (PA): ")) + wxString(info->name);
}

static bool has_initialised = false;

void assure_initialised() {
  if (! has_initialised) {
    Pa_Initialize();
    has_initialised = true;
  }
}

void GOrgueSoundPortaudioPort::terminate()
{
  if (has_initialised) {
    Pa_Terminate();
    has_initialised = false;
  }
}


GOrgueSoundPort* GOrgueSoundPortaudioPort::create(const GOrgueSoundPortsConfig &portsConfig, GOrgueSound* sound, wxString name)
{
  if (portsConfig.IsEnabled(PORT_NAME))
  {
    assure_initialised();
    for(int i = 0; i < Pa_GetDeviceCount(); i++)
    {
      wxString devName = getName(i);
	    if (devName == name || get_oldstyle_name(i) == name)
		    return new GOrgueSoundPortaudioPort(sound, devName);
    }
  }
  return NULL;
}

void GOrgueSoundPortaudioPort::addDevices(const GOrgueSoundPortsConfig &portsConfig, std::vector<GOrgueSoundDevInfo>& result)
{
  if (portsConfig.IsEnabled(PORT_NAME))
  {
    assure_initialised();
    for(int i = 0; i < Pa_GetDeviceCount(); i++)
    {
	    const PaDeviceInfo* dev_info = Pa_GetDeviceInfo(i);
	    if (dev_info->maxOutputChannels < 1)
		    continue;

	    GOrgueSoundDevInfo info;
	    info.channels = dev_info->maxOutputChannels;
	    info.isDefault = (Pa_GetDefaultOutputDevice() == i);
	    info.name = getName(i);
	    result.push_back(info);
    }
  }
}

