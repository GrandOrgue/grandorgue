/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueSoundPort.h"

#include "GOrgueSoundRtPort.h"
#include "GOrgueSoundPortaudioPort.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"

GOrgueSoundPort::GOrgueSoundPort(GOrgueSound* sound, wxString name) :
	m_Sound(sound),
	m_Index(0),
	m_IsOpen(false),
	m_Name(name),
	m_Channels(0),
	m_SamplesPerBuffer(0),
	m_SampleRate(0),
	m_Latency(0)

{
}

GOrgueSoundPort::~GOrgueSoundPort()
{
}

void GOrgueSoundPort::Init(unsigned channels, unsigned sample_rate, unsigned samples_per_buffer, unsigned latency, unsigned index)
{
	m_Index = index;
	m_Channels = channels;
	m_SampleRate = sample_rate;
	m_SamplesPerBuffer = samples_per_buffer;
	m_Latency = latency;
}

void GOrgueSoundPort::SetActualLatency(double latency)
{
	if (latency < m_SamplesPerBuffer / (double)m_SampleRate)
		latency = m_SamplesPerBuffer / (double)m_SampleRate;
	if (latency < 2 * m_SamplesPerBuffer / (double)m_SampleRate)
		latency += m_SamplesPerBuffer / (double)m_SampleRate;
	m_Sound->GetSettings().SetAudioDeviceActualLatency(m_Name, (int)(latency * 1000));
}

bool GOrgueSoundPort::AudioCallback(float* outputBuffer, unsigned int nFrames)
{
	return m_Sound->AudioCallback(m_Index, outputBuffer, nFrames);
}

const wxString& GOrgueSoundPort::GetName()
{
	return m_Name;
}

GOrgueSoundPort* GOrgueSoundPort::create(GOrgueSound* sound, wxString name)
{
	GOrgueSoundPort *port = NULL;
	if (port == NULL)
		port = GOrgueSoundPortaudioPort::create(sound, name);
	if (port == NULL)
		port = GOrgueSoundRtPort::create(sound, name);
	return port;
}

std::vector<GOrgueSoundDevInfo> GOrgueSoundPort::getDeviceList()
{
	std::vector<GOrgueSoundDevInfo> result;
	GOrgueSoundPortaudioPort::addDevices(result);
	GOrgueSoundRtPort::addDevices(result);
	return result;
}
