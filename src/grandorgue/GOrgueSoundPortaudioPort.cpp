/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueSoundPortaudioPort.h"

#include <wx/intl.h>

GOrgueSoundPortaudioPort::GOrgueSoundPortaudioPort(GOrgueSound* sound, wxString name) :
	GOrgueSoundPort(sound, name),
	m_stream(0)
{
}

GOrgueSoundPortaudioPort::~GOrgueSoundPortaudioPort()
{
	Close();
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
		throw wxString::Format(_("Open of the audio stream for %s failed: %s"), m_Name.c_str(), wxGetTranslation(wxString::FromAscii(Pa_GetErrorText(error))));
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
				       wxGetTranslation(wxString::FromAscii(Pa_GetErrorText(error))));

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

wxString GOrgueSoundPortaudioPort::getName(unsigned index)
{
	const PaDeviceInfo* info = Pa_GetDeviceInfo(index);
	const PaHostApiInfo *api = Pa_GetHostApiInfo(info->hostApi);
	return wxGetTranslation(wxString::FromAscii(api->name)) + wxString(_(" (PA): ")) + wxString::FromAscii(info->name);
}

GOrgueSoundPort* GOrgueSoundPortaudioPort::create(GOrgueSound* sound, wxString name)
{
	for(int i = 0; i < Pa_GetDeviceCount(); i++)
		if (getName(i) == name)
			return new GOrgueSoundPortaudioPort(sound, name);
	return NULL;
}

void GOrgueSoundPortaudioPort::addDevices(std::vector<GOrgueSoundDevInfo>& result)
{
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
