/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueSettings.h"

#include <wx/wx.h>
#include <wx/confbase.h>

GOrgueSettings::GOrgueSettings() :
	m_Config(*wxConfigBase::Get()),
	m_Stereo(false),
	m_Concurrency(0),
	m_ReleaseConcurrency(1),
	m_LosslessCompression(true),
	m_ManagePolyphony(true),
	m_CompressCache(true),
	m_ScaleRelease(true),
	m_RandomizeSpeaking(true),
	m_SampleRate(44100),
	m_WaveFormat(4),
	m_Volume(50),
	m_PolyphonyLimit(2048),
	m_DefaultAudioDevice()
{
}

wxConfigBase& GOrgueSettings::GetConfig()
{
	return m_Config;
}

void GOrgueSettings::Load()
{
	m_Stereo = m_Config.Read(wxT("StereoEnabled"), 1);
	m_Concurrency = m_Config.Read(wxT("Concurrency"), 0L);
	m_ReleaseConcurrency = m_Config.Read(wxT("ReleaseConcurrency"), 1L);
	if (m_ReleaseConcurrency < 1)
		m_ReleaseConcurrency = 1;
	m_LosslessCompression = m_Config.Read(wxT("LosslessCompression"), 1);
	m_ManagePolyphony = m_Config.Read(wxT("ManagePolyphony"), 1);
	m_CompressCache = m_Config.Read(wxT("CompressCache"), 1);
	m_ScaleRelease = m_Config.Read(wxT("ScaleRelease"), 1);
	m_RandomizeSpeaking = m_Config.Read(wxT("RandomizeSpeaking"), 1);
	m_SampleRate = m_Config.Read(wxT("SampleRate"), 44100);
	if (m_SampleRate < 1000)
		m_SampleRate = 44100;
	m_WaveFormat = m_Config.Read(wxT("WaveFormat"), 4);
	if (m_WaveFormat > 4)
		m_WaveFormat = 4;
	m_Volume = m_Config.Read(wxT("Volume"), 50);
	if (m_Volume > 100)
		m_Volume = 100;
	m_PolyphonyLimit = m_Config.Read(wxT("PolyphonyLimit"), 2048);
	m_DefaultAudioDevice = m_Config.Read(wxT("Devices/DefaultSound"), wxEmptyString);
}

bool GOrgueSettings::GetLoadInStereo()
{
	return m_Stereo;
}

void GOrgueSettings::SetLoadInStereo(bool stereo)
{
	m_Stereo = stereo;
	m_Config.Write(wxT("StereoEnabled"), m_Stereo);
}

unsigned GOrgueSettings::GetConcurrency()
{
	return m_Concurrency;
}

void GOrgueSettings::SetConcurrency(unsigned concurrency)
{
	m_Concurrency = concurrency;
	m_Config.Write(wxT("Concurrency"), (long)m_Concurrency);
}

unsigned GOrgueSettings::GetReleaseConcurrency()
{
	return m_ReleaseConcurrency;
}

void GOrgueSettings::SetReleaseConcurrency(unsigned concurrency)
{
	if (concurrency < 1)
		concurrency = 1;
	m_ReleaseConcurrency = concurrency;
	m_Config.Write(wxT("ReleaseConcurrency"), (long)m_ReleaseConcurrency);
}

bool GOrgueSettings::GetLosslessCompression()
{
	return m_LosslessCompression;
}

void GOrgueSettings::SetLosslessCompression(bool lossless_compression)
{
	m_LosslessCompression = lossless_compression;
	m_Config.Write(wxT("LosslessCompression"), m_LosslessCompression);
}

bool GOrgueSettings::GetManagePolyphony()
{
	return m_ManagePolyphony;
}

void GOrgueSettings::SetManagePolyphony(bool manage_polyphony)
{
	m_ManagePolyphony = manage_polyphony;
	m_Config.Write(wxT("ManagePolyphony"), m_ManagePolyphony);
}

bool GOrgueSettings::GetCompressCache()
{
	return m_CompressCache;
}

void GOrgueSettings::SetCompressCache(bool compress)
{
	m_CompressCache = compress;
	m_Config.Write(wxT("CompressCache"), m_CompressCache);
}

bool GOrgueSettings::GetScaleRelease()
{
	return m_ScaleRelease;
}

void GOrgueSettings::SetScaleRelease(bool scale_release)
{
	m_ScaleRelease = scale_release;
	m_Config.Write(wxT("ScaleRelease"), m_ScaleRelease);
}

bool GOrgueSettings::GetRandomizeSpeaking()
{
	return m_RandomizeSpeaking;
}

void GOrgueSettings::SetRandomizeSpeaking(bool randomize)
{
	m_RandomizeSpeaking = randomize;
	m_Config.Write(wxT("RandomizeSpeaking"), m_RandomizeSpeaking);
}

unsigned GOrgueSettings::GetSampleRate()
{
	return m_SampleRate;
}

void GOrgueSettings::SetSampleRate(unsigned sample_rate)
{
	if (sample_rate < 1000)
		sample_rate = 44100;
	m_SampleRate = sample_rate;
	m_Config.Write(wxT("SampleRate"), (long)sample_rate);
}

unsigned GOrgueSettings::GetWaveFormatBytesPerSample()
{
	return m_WaveFormat;
}

void GOrgueSettings::SetWaveFormatBytesPerSample(unsigned bytes_per_sample)
{
	if (bytes_per_sample > 4)
		bytes_per_sample = 4;
	m_WaveFormat = bytes_per_sample;
	m_Config.Write(wxT("WaveFormat"), (long)m_WaveFormat);
}

unsigned GOrgueSettings::GetVolume()
{
	return m_Volume;
}

void GOrgueSettings::SetVolume(unsigned volume)
{
	if (volume > 100)
		volume = 100;
	m_Volume = volume;
	m_Config.Write(wxT("Volume"), (long)m_Volume);
}

unsigned GOrgueSettings::GetPolyphonyLimit()
{
	return m_PolyphonyLimit;
}

void GOrgueSettings::SetPolyphonyLimit(unsigned polyphony_limit)
{
	m_PolyphonyLimit = polyphony_limit;
	m_Config.Write(wxT("PolyphonyLimit"), (long)m_PolyphonyLimit);
}

const wxString& GOrgueSettings::GetDefaultAudioDevice()
{
	return m_DefaultAudioDevice;
}

void GOrgueSettings::SetDefaultAudioDevice(wxString device)
{
	m_DefaultAudioDevice = device;
	m_Config.Write(wxT("Devices/DefaultSound"), m_DefaultAudioDevice);
}

unsigned GOrgueSettings::GetAudioDeviceLatency(wxString device)
{
	return m_Config.Read(wxT("Devices/Sound/") + device, 15L);
}

void GOrgueSettings::SetAudioDeviceLatency(wxString device, unsigned latency)
{
	m_Config.Write(wxT("Devices/Sound/") + device, (long) latency);
}

int GOrgueSettings::GetAudioDeviceActualLatency(wxString device)
{
	return m_Config.Read(wxT("Devices/Sound/ActualLatency/") + device, -1L);
}

void GOrgueSettings::SetAudioDeviceActualLatency(wxString device, unsigned latency)
{
	m_Config.Write(wxT("Devices/Sound/ActualLatency/") + device, (long) latency);
}
