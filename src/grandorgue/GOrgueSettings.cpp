/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "GOrgueSettings.h"

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/confbase.h>

const wxString GOrgueSettings::m_ManualNames[] = {
	wxTRANSLATE("Manual 1 (Pedal)"),
	wxTRANSLATE("Manual 2 (Great)"),
	wxTRANSLATE("Manual 3 (Swell)"),
	wxTRANSLATE("Manual 4 (Choir)"),
	wxTRANSLATE("Manual 5 (Solo)"),
	wxTRANSLATE("Manual 6 (Echo)"),
};

const wxString GOrgueSettings::m_EnclosureNames[] = {
	wxTRANSLATE("Enclosure 1"),
	wxTRANSLATE("Enclosure 2"),
	wxTRANSLATE("Enclosure 3"),
	wxTRANSLATE("Enclosure 4"),
	wxTRANSLATE("Enclosure 5"),
	wxTRANSLATE("Enclosure 6"),
};

const wxString GOrgueSettings::m_SetterNames[] = {
	wxTRANSLATE("Previous Memory"),
	wxTRANSLATE("Next Memory"),
	wxTRANSLATE("Memory Set"),
};

const wxString GOrgueSettings::m_StopChangeName = wxTRANSLATE("Stop Changes");


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
	m_BitsPerSample(16),
	m_InterpolationType(0),
	m_WaveFormat(4),
	m_LoopLoad(0),
	m_AttackLoad(0),
	m_ReleaseLoad(0),
	m_Volume(50),
	m_PolyphonyLimit(2048),
	m_DefaultAudioDevice(),
	m_Preset(0),
	m_OrganMidiEvents(),
	m_WAVPath(),
	m_OrganPath(),
	m_SettingPath(),
	m_UserSettingPath(),
	m_UserCachePath(),
	m_ManualEvents(),
	m_EnclosureEvents(),
	m_SetterEvents(),
	m_StopChangeEvent(0),
	m_Transpose(0)
{
	GetConfig().SetRecordDefaults();
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
	m_LosslessCompression = m_Config.Read(wxT("LosslessCompression"), 0L);
	m_ManagePolyphony = m_Config.Read(wxT("ManagePolyphony"), 1);
	m_CompressCache = m_Config.Read(wxT("CompressCache"), 1);
	m_ScaleRelease = m_Config.Read(wxT("ScaleRelease"), 1);
	m_RandomizeSpeaking = m_Config.Read(wxT("RandomizeSpeaking"), 1);
	m_SampleRate = m_Config.Read(wxT("SampleRate"), 44100);
	if (m_SampleRate < 1000)
		m_SampleRate = 44100;
	m_BitsPerSample = m_Config.Read(wxT("BitsPerSample"), 2);
	if (m_BitsPerSample != 8 && m_BitsPerSample != 12 && m_BitsPerSample != 16 && m_BitsPerSample != 20 && m_BitsPerSample != 24)
		m_BitsPerSample = 16;
	m_InterpolationType = m_Config.Read(wxT("InterpolationType"), 1);
	if (m_InterpolationType > 1)
		m_InterpolationType = 0;
	m_WaveFormat = m_Config.Read(wxT("WaveFormat"), 4);
	if (m_WaveFormat < 1 || m_WaveFormat > 4)
		m_WaveFormat = 4;
	m_LoopLoad = m_Config.Read(wxT("LoopLoad"), 2);
	if (m_LoopLoad < 0 || m_LoopLoad > 2)
		m_LoopLoad = 2;
	m_AttackLoad = m_Config.Read(wxT("AttackLoad"), 1);
	if (m_AttackLoad < 0 || m_AttackLoad > 1)
		m_AttackLoad = 1;
	m_ReleaseLoad = m_Config.Read(wxT("ReleaseLoad"), 1);
	if (m_ReleaseLoad < 0 || m_ReleaseLoad > 1)
		m_ReleaseLoad = 1;
	m_Volume = m_Config.Read(wxT("Volume"), 50);
	if (m_Volume > 100)
		m_Volume = 100;
	m_PolyphonyLimit = m_Config.Read(wxT("PolyphonyLimit"), 2048);
	m_DefaultAudioDevice = m_Config.Read(wxT("Devices/DefaultSound"), wxEmptyString);

	m_OrganMidiEvents.clear();
	long count = m_Config.Read(wxT("OrganMIDI/Count"), 0L);
	for (long i = 0; i < count; i++)
	{
		wxString itemstr = wxT("OrganMIDI/Organ") + wxString::Format(wxT("%ld"), i);
		long j = m_Config.Read(itemstr + wxT(".midi"), 0L);
		wxString file = m_Config.Read(itemstr + wxT(".file"));
		m_OrganMidiEvents.insert(std::pair<long, wxString>(j, file));
	}
	m_WAVPath = m_Config.Read(wxT("wavPath"), GetStandardDocumentDirectory());
	m_OrganPath = m_Config.Read(wxT("organPath"), GetStandardOrganDirectory());
	m_SettingPath = m_Config.Read(wxT("cmbPath"), GetStandardOrganDirectory());
	SetUserSettingPath (m_Config.Read(wxT("SettingPath"), wxEmptyString));
	SetUserCachePath (m_Config.Read(wxT("CachePath"), wxEmptyString));
	m_Preset = m_Config.Read(wxT("Preset"), 0L);

	m_StopChangeEvent = m_Config.Read(wxString(wxT("MIDI/")) + m_StopChangeName, 0x9400);
	for(unsigned i = 0; i < GetManualCount(); i++)
		m_ManualEvents[i] = m_Config.Read(wxString(wxT("MIDI/")) + m_ManualNames[i], 0x9000 + (((i < 4 ? i : i + 1) << 8) & 0x0F00));

	for(unsigned i = 0; i < GetEnclosureCount(); i++)
		m_EnclosureEvents[i] = m_Config.Read(wxString(wxT("MIDI/")) + m_EnclosureNames[i], 0x0000L);

	for(unsigned i = 0; i < GetSetterCount(); i++)
		m_SetterEvents[i] = m_Config.Read(wxString(wxT("MIDI/")) + m_SetterNames[i], i < 2 ? (0xC400 + i) : 0x0000);
}

unsigned GOrgueSettings::GetManualCount()
{
	return sizeof(m_ManualEvents) / sizeof(m_ManualEvents[0]);
}

wxString GOrgueSettings::GetManualTitle(unsigned index)
{
	assert(index < GetManualCount());
	return wxGetTranslation(m_ManualNames[index]);
}

int GOrgueSettings::GetManualEvent(unsigned index)
{
	assert(index < GetManualCount());
	return m_ManualEvents[index];
}

void GOrgueSettings::SetManualEvent(unsigned index, int event)
{
	assert(index < GetManualCount());
	m_ManualEvents[index] = event;
	m_Config.Write(wxString(wxT("MIDI/")) + m_ManualNames[index], m_ManualEvents[index]);
}

unsigned GOrgueSettings::GetEnclosureCount()
{
	return sizeof(m_EnclosureEvents) / sizeof(m_EnclosureEvents[0]);
}

wxString GOrgueSettings::GetEnclosureTitle(unsigned index)
{
	assert(index < GetEnclosureCount());
	return wxGetTranslation(m_EnclosureNames[index]);
}

int GOrgueSettings::GetEnclosureEvent(unsigned index)
{
	assert(index < GetEnclosureCount());
	return m_EnclosureEvents[index];
}

void GOrgueSettings::SetEnclosureEvent(unsigned index, int event)
{
	assert(index < GetEnclosureCount());
	m_EnclosureEvents[index] = event;
	m_Config.Write(wxString(wxT("MIDI/")) + m_EnclosureNames[index], m_EnclosureEvents[index]);
}

unsigned GOrgueSettings::GetSetterCount()
{
	return sizeof(m_SetterEvents) / sizeof(m_SetterEvents[0]);
}

wxString GOrgueSettings::GetSetterTitle(unsigned index)
{
	assert(index < GetSetterCount());
	return wxGetTranslation(m_SetterNames[index]);
}

int GOrgueSettings::GetSetterEvent(unsigned index)
{
	assert(index < GetSetterCount());
	return m_SetterEvents[index];
}

void GOrgueSettings::SetSetterEvent(unsigned index, int event)
{
	assert(index < GetSetterCount());
	m_SetterEvents[index] = event;
	m_Config.Write(wxString(wxT("MIDI/")) + m_SetterNames[index], m_SetterEvents[index]);
}

wxString GOrgueSettings::GetStopChangeTitle()
{
	return wxGetTranslation(m_StopChangeName);
}

int GOrgueSettings::GetStopChangeEvent()
{
	return m_StopChangeEvent;
}

void GOrgueSettings::SetStopChangeEvent(int event)
{
	m_StopChangeEvent = event;
	m_Config.Write(wxString(wxT("MIDI/")) + m_StopChangeName, m_StopChangeEvent);
}

wxString GOrgueSettings::GetStandardDocumentDirectory()
{
	return wxStandardPaths::Get().GetDocumentsDir();
}

wxString GOrgueSettings::GetStandardOrganDirectory()
{
	return GetStandardDocumentDirectory() + wxFileName::GetPathSeparator() + _("My Organs");
}

wxString GOrgueSettings::GetStandardDataDirectory()
{
	return wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueData");
}

wxString GOrgueSettings::GetStandardCacheDirectory()
{
	return wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueCache");
}

wxString GOrgueSettings::GetOrganPath()
{
	return m_OrganPath;
}

void GOrgueSettings::SetOrganPath(wxString path)
{
	m_OrganPath = path;
	m_Config.Write(wxT("organPath"), m_OrganPath);
}

wxString GOrgueSettings::GetSettingPath()
{
	return m_SettingPath;
}

void GOrgueSettings::SetSettingPath(wxString path)
{
	m_SettingPath = path;
	m_Config.Write(wxT("cmbPath"), m_SettingPath);
}

wxString GOrgueSettings::GetWAVPath()
{
	return m_WAVPath;
}

void GOrgueSettings::SetWAVPath(wxString path)
{
	m_WAVPath = path;
	m_Config.Write(wxT("wavPath"), m_WAVPath);
}

wxString GOrgueSettings::GetUserSettingPath()
{
	return m_UserSettingPath;
}

void GOrgueSettings::SetUserSettingPath(wxString path)
{
	if (path == wxEmptyString)
		path = GetStandardDataDirectory();
	if (!wxFileName::DirExists(path))
		path = GetStandardDataDirectory();
	wxFileName file(path);
	file.MakeAbsolute();
	path = file.GetFullPath();
	if (!wxFileName::DirExists(path))
		wxFileName::Mkdir(path);
	m_UserSettingPath = path;
	m_Config.Write(wxT("SettingPath"), m_UserSettingPath);
}

wxString GOrgueSettings::GetUserCachePath()
{
	return m_UserCachePath;
}

void GOrgueSettings::SetUserCachePath(wxString path)
{
	if (path == wxEmptyString)
		path = GetStandardCacheDirectory();
	if (!wxFileName::DirExists(path))
		path = GetStandardCacheDirectory();
	wxFileName file(path);
	file.MakeAbsolute();
	path = file.GetFullPath();
	if (!wxFileName::DirExists(path))
		wxFileName::Mkdir(path);
	m_UserCachePath = path;
	m_Config.Write(wxT("CachePath"), m_UserCachePath);
}

unsigned  GOrgueSettings::GetPreset()
{
	return m_Preset;
}

void  GOrgueSettings::SetPreset(unsigned value)
{
	m_Preset = value;
	m_Config.Write(wxT("Preset"), (long)m_Preset);
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

unsigned GOrgueSettings::GetInterpolationType()
{
	return m_InterpolationType;
}

void GOrgueSettings::SetInterpolationType(unsigned type)
{
	if (type > 1)
		type = 0;
	m_InterpolationType = type;
	m_Config.Write(wxT("InterpolationType"), (long)m_InterpolationType);
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

unsigned GOrgueSettings::GetBitsPerSample()
{
	return m_BitsPerSample;
}

void GOrgueSettings::SetBitsPerSample(unsigned bits_per_sample)
{
	if (bits_per_sample != 8 && bits_per_sample != 12 && bits_per_sample != 16 && bits_per_sample != 20 && bits_per_sample != 24)
		bits_per_sample = 16;
	m_BitsPerSample = bits_per_sample;
	m_Config.Write(wxT("BitsPerSample"), (long)m_BitsPerSample);
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

unsigned GOrgueSettings::GetLoopLoad()
{
	return m_LoopLoad;
}

void GOrgueSettings::SetLoopLoad(unsigned loop_load)
{
	if (loop_load > 2)
		loop_load = 2;
	m_LoopLoad = loop_load;
	m_Config.Write(wxT("LoopLoad"), (long)m_LoopLoad);
}

unsigned GOrgueSettings::GetAttackLoad()
{
	return m_AttackLoad;
}

void GOrgueSettings::SetAttackLoad(unsigned attack_load)
{
	if (attack_load > 1)
		attack_load = 1;
	m_AttackLoad = attack_load;
	m_Config.Write(wxT("AttackLoad"), (long)m_AttackLoad);
}

unsigned GOrgueSettings::GetReleaseLoad()
{
	return m_ReleaseLoad;
}

void GOrgueSettings::SetReleaseLoad(unsigned release_load)
{
	if (release_load > 1)
		release_load = 1;
	m_ReleaseLoad = release_load;
	m_Config.Write(wxT("ReleaseLoad"), (long)m_ReleaseLoad);
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

const std::map<long, wxString>& GOrgueSettings::GetOrganList()
{
	return m_OrganMidiEvents;
}

void GOrgueSettings::SetOrganList(std::map<long, wxString> list)
{
	m_OrganMidiEvents = list;
	unsigned count = 0;
	for(std::map<long, wxString>::iterator it = m_OrganMidiEvents.begin(); it != m_OrganMidiEvents.end(); it++, count++)
	{ 
		wxString itemstr = wxString::Format(wxT("OrganMIDI/Organ%d"), count);
		m_Config.Write(itemstr+wxT(".file"), it->second);
		m_Config.Write(itemstr+wxT(".midi"), it->first);
	}
	m_Config.Write(wxT("OrganMIDI/Count"), (long)count);
}

int GOrgueSettings::GetMidiDeviceChannelShift(wxString device)
{
	return m_Config.Read(wxT("Devices/MIDI/") + device, 0L);
}

void GOrgueSettings::SetMidiDeviceChannelShift(wxString device, int shift)
{
	m_Config.Write(wxT("Devices/MIDI/") + device, shift);
}

std::vector<wxString> GOrgueSettings::GetMidiDeviceList()
{
	std::vector<wxString> list;
	wxString str;
	long no;

	m_Config.SetPath(wxT("/Devices/MIDI"));
	if (m_Config.GetFirstEntry(str, no))
		do
		{
			list.push_back(str);
		}
		while (m_Config.GetNextEntry(str, no));

	m_Config.SetPath(wxT("/"));
	return list;
}

int GOrgueSettings::GetTranspose()
{
	return m_Transpose;
}

void GOrgueSettings::SetTranspose(int transpose)
{
	m_Transpose = transpose;
}

