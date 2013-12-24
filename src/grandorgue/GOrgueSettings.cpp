/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueSettings.h"

#include "GOSoundDefs.h"
#include "GOrgueConfigFileWriter.h"
#include "GOrgueConfigFileReader.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigReaderDB.h"
#include "GOrgueConfigWriter.h"
#include "GOrguePath.h"
#include <wx/confbase.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/thread.h>
#include <algorithm>

const GOMidiSetting GOrgueSettings:: m_MIDISettings[] = {
	{ MIDI_RECV_MANUAL, 1, wxTRANSLATE("Manuals"), wxTRANSLATE("Pedal") },
	{ MIDI_RECV_MANUAL, 2, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 1") },
	{ MIDI_RECV_MANUAL, 3, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 2") },
	{ MIDI_RECV_MANUAL, 4, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 3") },
	{ MIDI_RECV_MANUAL, 5, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 4") },
	{ MIDI_RECV_MANUAL, 6, wxTRANSLATE("Manuals"), wxTRANSLATE("Manual 5") },
	{ MIDI_RECV_ENCLOSURE, 1, wxTRANSLATE("Enclosures"), wxTRANSLATE("Enclosure 1") },
	{ MIDI_RECV_ENCLOSURE, 2, wxTRANSLATE("Enclosures"), wxTRANSLATE("Enclosure 2") },
	{ MIDI_RECV_ENCLOSURE, 3, wxTRANSLATE("Enclosures"), wxTRANSLATE("Enclosure 3") },
	{ MIDI_RECV_ENCLOSURE, 4, wxTRANSLATE("Enclosures"), wxTRANSLATE("Enclosure 4") },
	{ MIDI_RECV_ENCLOSURE, 5, wxTRANSLATE("Enclosures"), wxTRANSLATE("Enclosure 5") },
	{ MIDI_RECV_ENCLOSURE, 6, wxTRANSLATE("Enclosures"), wxTRANSLATE("Enclosure 6") },
	{ MIDI_RECV_SETTER, 0, wxTRANSLATE("Sequencer"), wxTRANSLATE("Previous Memory") },
	{ MIDI_RECV_SETTER, 1, wxTRANSLATE("Sequencer"), wxTRANSLATE("Next Memory") },
	{ MIDI_RECV_SETTER, 2, wxTRANSLATE("Sequencer"), wxTRANSLATE("Memory Set") },
	{ MIDI_RECV_SETTER, 3, wxTRANSLATE("Sequencer"), wxTRANSLATE("Current") },
	{ MIDI_RECV_SETTER, 4, wxTRANSLATE("Sequencer"), wxTRANSLATE("G.C.") },
	{ MIDI_RECV_SETTER, 5, wxTRANSLATE("Sequencer"), wxTRANSLATE("-10") },
	{ MIDI_RECV_SETTER, 6, wxTRANSLATE("Sequencer"), wxTRANSLATE("+10") },
	{ MIDI_RECV_SETTER, 7, wxTRANSLATE("Sequencer"), wxTRANSLATE("__0") },
	{ MIDI_RECV_SETTER, 8, wxTRANSLATE("Sequencer"), wxTRANSLATE("__1") },
	{ MIDI_RECV_SETTER, 9, wxTRANSLATE("Sequencer"), wxTRANSLATE("__2") },
	{ MIDI_RECV_SETTER, 10, wxTRANSLATE("Sequencer"), wxTRANSLATE("__3") },
	{ MIDI_RECV_SETTER, 11, wxTRANSLATE("Sequencer"), wxTRANSLATE("__4") },
	{ MIDI_RECV_SETTER, 12, wxTRANSLATE("Sequencer"), wxTRANSLATE("__5") },
	{ MIDI_RECV_SETTER, 13, wxTRANSLATE("Sequencer"), wxTRANSLATE("__6") },
	{ MIDI_RECV_SETTER, 14, wxTRANSLATE("Sequencer"), wxTRANSLATE("__7") },
	{ MIDI_RECV_SETTER, 15, wxTRANSLATE("Sequencer"), wxTRANSLATE("__8") },
	{ MIDI_RECV_SETTER, 16, wxTRANSLATE("Sequencer"), wxTRANSLATE("__9") },
	{ MIDI_RECV_SETTER, 17, wxTRANSLATE("Master Controls"), wxTRANSLATE("-1 Cent") },
	{ MIDI_RECV_SETTER, 18, wxTRANSLATE("Master Controls"), wxTRANSLATE("+1 Cent") },
	{ MIDI_RECV_SETTER, 19, wxTRANSLATE("Master Controls"), wxTRANSLATE("-100 Cent") },
	{ MIDI_RECV_SETTER, 20, wxTRANSLATE("Master Controls"), wxTRANSLATE("+100 Cent") },
	{ MIDI_RECV_SETTER, 21, wxTRANSLATE("Master Controls"), wxTRANSLATE("Prev temperament") },
	{ MIDI_RECV_SETTER, 22, wxTRANSLATE("Master Controls"), wxTRANSLATE("Next temperament") },
	{ MIDI_RECV_SETTER, 23, wxTRANSLATE("Master Controls"), wxTRANSLATE("Transpose -") },
	{ MIDI_RECV_SETTER, 24, wxTRANSLATE("Master Controls"), wxTRANSLATE("Transpose +") },
};

GOrgueSettings::GOrgueSettings(wxString instance) :
	m_Config(*wxConfigBase::Get()),
	m_InstanceName(instance),
	m_MemoryLimit(0),
	m_Stereo(false),
	m_Concurrency(0),
	m_ReleaseConcurrency(1),
	m_LoadConcurrency(0),
	m_LosslessCompression(true),
	m_ManagePolyphony(true),
	m_ManageCache(true),
	m_CompressCache(false),
	m_ScaleRelease(true),
	m_RandomizeSpeaking(true),
	m_LoadLastFile(true),
	m_SampleRate(44100),
	m_BitsPerSample(24),
	m_InterpolationType(0),
	m_WaveFormat(4),
	m_LoopLoad(0),
	m_AttackLoad(0),
	m_ReleaseLoad(0),
	m_Volume(-15),
	m_PolyphonyLimit(2048),
	m_Preset(0),
	m_OrganList(),
	m_WAVPath(),
	m_OrganPath(),
	m_SettingPath(),
	m_UserSettingPath(),
	m_UserCachePath(),
	m_LastFile(),
	m_AudioGroups(),
	m_AudioDeviceConfig(),
	m_Transpose(0),
	m_Reverb(0),
	m_ReverbEnabled(false),
	m_ReverbDirect(true),
	m_ReverbChannel(1),
	m_ReverbStartOffset(0),
	m_ReverbLen(0),
	m_ReverbDelay(0),
	m_ReverbGain(1),
	m_ReverbFile(),
	m_MIDIEvents()
{
	GetConfig().SetRecordDefaults();
	m_ConfigFileName = wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueConfig") + m_InstanceName;
	for(unsigned i = 0; i < GetEventCount(); i++)
		m_MIDIEvents.push_back(new GOrgueMidiReceiver(NULL, m_MIDISettings[i].type));
}

GOrgueSettings::~GOrgueSettings()
{
	Flush();
}

wxConfigBase& GOrgueSettings::GetConfig()
{
	return m_Config;
}

void GOrgueSettings::Load()
{
	if (wxFileExists(m_ConfigFileName))
	{
		try
		{
			GOrgueConfigFileReader cfg_file;
			if (!cfg_file.Read(m_ConfigFileName))
				throw wxString::Format(_("Unable to read '%s'"), m_ConfigFileName.c_str());

			GOrgueConfigReaderDB cfg_db;
			cfg_db.ReadData(cfg_file, CMBSetting, false);
			GOrgueConfigReader cfg(cfg_db);

			long cpus = wxThread::GetCPUCount();
			if (cpus == -1)
				cpus = 4;
			if (cpus >= 256)
				cpus = 256;
			if (cpus == 0)
				cpus = 1;
			SetConcurrency(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("Concurrency"), 0, 256, false, cpus));
			SetReleaseConcurrency(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("ReleaseConcurrency"), 1, 256, false, cpus));
			SetLoadConcurrency(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("LoadConcurrency"), 0, 256, false, cpus));

			SetInterpolationType(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("InterpolationType"), 0, 1, false, 1));
			SetWaveFormatBytesPerSample(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("WaveFormat"), 1, 4, false, 4));
			SetAttackLoad(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("AttackLoad"), 0, 1, false, 1));
			SetLoopLoad(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("LoopLoad"), 0, 2, false, 2));
			SetReleaseLoad(cfg.ReadInteger(CMBSetting, wxT("General"), wxT("ReleaseLoad"), 0, 1, false, 1));

			SetManageCache(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("ManageCache"), false, true));
			SetCompressCache(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("CompressCache"), false, false));
			SetLoadLastFile(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("LoadLastFile"), false, true));
			SetLastFile(cfg.ReadString(CMBSetting, wxT("General"), wxT("LastFile"), 512, false, wxEmptyString));

			SetLoadInStereo(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("Stereo"), false, true));
			SetLosslessCompression(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("LosslessCompression"), false, false));
			SetManagePolyphony(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("ManagePolyphony"), false, true));
			SetScaleRelease(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("ScaleRelease"), false, true));
			SetRandomizeSpeaking(cfg.ReadBoolean(CMBSetting, wxT("General"), wxT("RandomizeSpeaking"), false, true));
			SetMemoryLimit(cfg.ReadFloat(CMBSetting, wxT("General"), wxT("MemoryLimit"), 0, 1024 * 1024, false, 0) * (1024.0 * 1024.0));

			SetReverbEnabled(cfg.ReadBoolean(CMBSetting, wxT("Reverb"), wxT("ReverbEnabled"), false, false));
			SetReverbDirect(cfg.ReadBoolean(CMBSetting, wxT("Reverb"), wxT("ReverbDirect"), false, true));
			SetReverbChannel(cfg.ReadInteger(CMBSetting, wxT("Reverb"), wxT("ReverbChannel"), 1, 4, false, 1));
			SetReverbStartOffset(cfg.ReadInteger(CMBSetting, wxT("Reverb"), wxT("ReverbStartOffset"), 0, 158760000, false, 0));
			SetReverbLen(cfg.ReadInteger(CMBSetting, wxT("Reverb"), wxT("ReverbLen"), 0, 158760000, false, 0));
			SetReverbDelay(cfg.ReadInteger(CMBSetting, wxT("Reverb"), wxT("ReverbDelay"), 0, 10000, false, 0));
			SetReverbGain(cfg.ReadFloat(CMBSetting, wxT("Reverb"), wxT("ReverbGain"), 0, 50, false, 1));
			SetReverbFile(cfg.ReadString(CMBSetting, wxT("Reverb"), wxT("ReverbFile"), 512, false, wxEmptyString));
		
			m_OrganList.clear();
			unsigned organ_count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("OrganCount"), 0, 99999, false, 0);
			for(unsigned i = 0; i < organ_count; i++)
				m_OrganList.push_back(new GOrgueOrgan(cfg, wxString::Format(wxT("Organ%03d"), i + 1)));

			for(unsigned i = 0; i < GetEventCount(); i++)
				m_MIDIEvents[i]->Load(cfg, GetEventSection(i));

			m_AudioGroups.clear();
			unsigned count = cfg.ReadInteger(CMBSetting, wxT("AudioGroups"), wxT("Count"), 0, 200, false, 0);
			for(unsigned i = 0; i < count; i++)
				m_AudioGroups.push_back(cfg.ReadString(CMBSetting, wxT("AudioGroups"), wxString::Format(wxT("Name%03d"), i + 1), 4096, false, wxString::Format(_("Audio group %d"), i + 1)));

			m_AudioDeviceConfig.clear();
			count = cfg.ReadInteger(CMBSetting, wxT("AudioDevices"), wxT("Count"), 0, 200, false, 0);
			for(unsigned i = 0; i < count; i++)
			{
				GOAudioDeviceConfig conf;
				conf.name = cfg.ReadString(CMBSetting, wxT("AudioDevices"), wxString::Format(wxT("Device%03dName"), i + 1));
				conf.channels = cfg.ReadInteger(CMBSetting, wxT("AudioDevices"), wxString::Format(wxT("Device%03dChannelCount"), i + 1), 0, 200);
				conf.scale_factors.resize(conf.channels);
				for(unsigned j = 0; j < conf.channels; j++)
				{
					wxString prefix = wxString::Format(wxT("Device%03dChannel%03d"), i + 1, j + 1);
					unsigned group_count = cfg.ReadInteger(CMBSetting, wxT("AudioDevices"), prefix + wxT("GroupCount"), 0, 200);
					for(unsigned k = 0; k < group_count; k++)
					{
						GOAudioGroupOutputConfig group;
						wxString p = prefix + wxString::Format(wxT("Group%03d"), k + 1);

						group.name = cfg.ReadString(CMBSetting, wxT("AudioDevices"), p + wxT("Name"));
						group.left = cfg.ReadFloat(CMBSetting, wxT("AudioDevices"), p + wxT("Left"), -121.0, 40);
						group.right = cfg.ReadFloat(CMBSetting, wxT("AudioDevices"), p + wxT("Right"), -121.0, 40);

						conf.scale_factors[j].push_back(group);
					}
				}
				m_AudioDeviceConfig.push_back(conf);
			}

			wxCopyFile(m_ConfigFileName, m_ConfigFileName + wxT(".last"));
		}
		catch (wxString error)
		{
			wxLogError(wxT("%s\n"),error.c_str());
		}
	}

	SetSamplesPerBuffer(m_Config.Read(wxT("SamplesPerBuffer"), 1024));
	m_SampleRate = m_Config.Read(wxT("SampleRate"), 44100);
	m_Reverb = m_Config.Read(wxT("Reverb"), 0L);
	if (m_Reverb > 5 || m_Reverb < 0)
		m_Reverb = 0;
	if (m_SampleRate < 1000)
		m_SampleRate = 44100;
	m_BitsPerSample = m_Config.Read(wxT("BitsPerSample"), 24);
	if (m_BitsPerSample != 8 && m_BitsPerSample != 12 && m_BitsPerSample != 16 && m_BitsPerSample != 20 && m_BitsPerSample != 24)
		m_BitsPerSample = 24;
	m_Volume = m_Config.Read(wxT("Volume"), (long)-15);
	if (m_Volume > 20)
		m_Volume = -15;
	m_PolyphonyLimit = m_Config.Read(wxT("PolyphonyLimit"), 2048);

	m_WAVPath = m_Config.Read(wxT("wavPath"), GetStandardDocumentDirectory());
	m_OrganPath = m_Config.Read(wxT("organPath"), GetStandardOrganDirectory());
	m_SettingPath = m_Config.Read(wxT("cmbPath"), GetStandardOrganDirectory());
	SetUserSettingPath (m_Config.Read(wxT("SettingPath"), wxEmptyString));
	SetUserCachePath (m_Config.Read(wxT("CachePath"), wxEmptyString));
	m_Preset = m_Config.Read(wxT("Preset"), 0L);

	if (m_AudioGroups.size() == 0)
	{
		unsigned count = m_Config.Read(wxT("AudioGroup/Count"), 0L);
		for(unsigned i = 0; i < count; i++)
			m_AudioGroups.push_back(m_Config.Read(wxString::Format(wxT("AudioGroup/Name%d"), i + 1), wxString::Format(_("Audio group %d"), i + 1)));
		if (!m_AudioGroups.size())
			m_AudioGroups.push_back(_("Default audio group"));
	}

	if (!m_AudioDeviceConfig.size())
	{
		unsigned count = m_Config.Read(wxT("AudioDevices/Count"), 0L);
		for(unsigned i = 0; i < count; i++)
		{
			GOAudioDeviceConfig conf;
			conf.name = m_Config.Read(wxString::Format(wxT("AudioDevice/Device%d/Name"), i + 1), wxEmptyString);
			conf.channels = m_Config.Read(wxString::Format(wxT("AudioDevice/Device%d/Channels"), i + 1), 2L);
			conf.scale_factors.resize(conf.channels);
			for(unsigned j = 0; j < conf.channels; j++)
			{
				wxString prefix = wxString::Format(wxT("AudioDevice/Device%d/Channel%d/"), i + 1, j + 1);
				unsigned group_count = m_Config.Read(prefix + wxT("GroupCount"), 0L);
				for(unsigned k = 0; k < group_count; k++)
				{
					GOAudioGroupOutputConfig group;
					wxString p = prefix + wxString::Format(wxT("Group%d/"), k + 1);

					group.name = m_Config.Read(p + wxT("Name"), wxEmptyString);
					double tmp;
					m_Config.Read(p + wxT("Left"), &tmp, -121.0f);
					if (tmp > 40.0 || tmp < -121.0)
						tmp = 0;
					group.left = tmp;
					m_Config.Read(p + wxT("Right"), &tmp, -121.0f);
					if (tmp > 40.0 || tmp < -121.0)
						tmp = 0;
					group.right = tmp;

					conf.scale_factors[j].push_back(group);
				}
			}
			m_AudioDeviceConfig.push_back(conf);
		}
	}
	if (!m_AudioDeviceConfig.size())
	{
		GOAudioDeviceConfig conf;
		conf.name = wxEmptyString;
		conf.channels = 2;
		conf.scale_factors.resize(conf.channels);
		for(unsigned k = 0; k < m_AudioGroups.size(); k++)
		{
			GOAudioGroupOutputConfig group;
			group.name = m_AudioGroups[k];

			group.left = 0.0f;
			group.right = -121.0f;
			conf.scale_factors[0].push_back(group);

			group.left = -121.0f;
			group.right = 0.0f;
			conf.scale_factors[1].push_back(group);
		}
		m_AudioDeviceConfig.push_back(conf);
	}

	wxString str;
	long no;
	m_Config.SetPath(wxT("/Devices/MIDI"));
	if (m_Config.GetFirstEntry(str, no))
		do
		{
			m_MidiIn[str] = m_Config.Read(str, 0L);
		}
		while (m_Config.GetNextEntry(str, no));
	m_Config.SetPath(wxT("/"));

	m_Config.SetPath(wxT("/Devices/MIDIOut"));
	if (m_Config.GetFirstEntry(str, no))
		do
		{
			m_MidiOut[str] = m_Config.Read(str, 0L) == 1 ? true : false;
		}
		while (m_Config.GetNextEntry(str, no));
	m_Config.SetPath(wxT("/"));
}

unsigned GOrgueSettings::GetEventCount()
{
	return sizeof(m_MIDISettings) / sizeof(m_MIDISettings[0]);
}

wxString GOrgueSettings::GetEventSection(unsigned index)
{
	assert(index < GetEventCount());
	switch(m_MIDISettings[index].type)
	{
	case MIDI_RECV_ENCLOSURE:
		return wxString::Format(wxT("Enclosure%03d"), index);

	case MIDI_RECV_MANUAL:
		return wxString::Format(wxT("Manual%03d"), index);

	case MIDI_RECV_SETTER:
		return wxString::Format(wxT("Setter%03d"), index);
		
	default:
	assert(false);
	return wxEmptyString;
	}
}

wxString GOrgueSettings::GetEventGroup(unsigned index)
{
	assert(index < GetEventCount());
	return wxGetTranslation(m_MIDISettings[index].group);
}

wxString GOrgueSettings::GetEventTitle(unsigned index)
{
	assert(index < GetEventCount());
	return wxGetTranslation(m_MIDISettings[index].name);
}

GOrgueMidiReceiver* GOrgueSettings::GetMidiEvent(unsigned index)
{
	assert(index < GetEventCount());
	return m_MIDIEvents[index];
}

GOrgueMidiReceiver* GOrgueSettings::FindMidiEvent(MIDI_RECEIVER_TYPE type, unsigned index)
{
	for(unsigned i = 0; i < GetEventCount(); i++)
		if (m_MIDISettings[i].type == type && m_MIDISettings[i].index == index)
			return m_MIDIEvents[i];
	return NULL;
}

size_t GOrgueSettings::GetMemoryLimit()
{
	return m_MemoryLimit;
}

void GOrgueSettings::SetMemoryLimit(size_t limit)
{
	m_MemoryLimit = limit;
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
	return wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueData") + m_InstanceName;
}

wxString GOrgueSettings::GetStandardCacheDirectory()
{
	return wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueCache") + m_InstanceName;
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
	GOCreateDirectory(path);
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
	GOCreateDirectory(path);
	m_UserCachePath = path;
	m_Config.Write(wxT("CachePath"), m_UserCachePath);
}

wxString GOrgueSettings::GetLastFile()
{
	return m_LastFile;
}

void GOrgueSettings::SetLastFile(wxString path)
{
	wxFileName file(path);
	file.MakeAbsolute();
	path = file.GetFullPath();
	m_LastFile = path;
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
}

unsigned GOrgueSettings::GetSamplesPerBuffer()
{
	return m_SamplesPerBuffer;
}

void GOrgueSettings::SetSamplesPerBuffer(unsigned sampler_per_buffer)
{
	m_SamplesPerBuffer = BLOCKS_PER_FRAME * ((sampler_per_buffer + BLOCKS_PER_FRAME - 1) / BLOCKS_PER_FRAME);
	if (m_SamplesPerBuffer > MAX_FRAME_SIZE)
		m_SamplesPerBuffer = MAX_FRAME_SIZE;
	if (m_SamplesPerBuffer < BLOCKS_PER_FRAME)
		m_SamplesPerBuffer = BLOCKS_PER_FRAME;
	m_Config.Write(wxT("SamplesPerBuffer"), (long)m_SamplesPerBuffer);
}

unsigned GOrgueSettings::GetConcurrency()
{
	return m_Concurrency;
}

void GOrgueSettings::SetConcurrency(unsigned concurrency)
{
	m_Concurrency = concurrency;
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
}

unsigned GOrgueSettings::GetLoadConcurrency()
{
	return m_LoadConcurrency;
}

void GOrgueSettings::SetLoadConcurrency(unsigned concurrency)
{
	m_LoadConcurrency = concurrency;
}

bool GOrgueSettings::GetLosslessCompression()
{
	return m_LosslessCompression;
}

void GOrgueSettings::SetLosslessCompression(bool lossless_compression)
{
	m_LosslessCompression = lossless_compression;
}

bool GOrgueSettings::GetManagePolyphony()
{
	return m_ManagePolyphony;
}

void GOrgueSettings::SetManagePolyphony(bool manage_polyphony)
{
	m_ManagePolyphony = manage_polyphony;
}

bool GOrgueSettings::GetManageCache()
{
	return m_ManageCache;
}

void GOrgueSettings::SetManageCache(bool manage)
{
	m_ManageCache = manage;
}

bool GOrgueSettings::GetCompressCache()
{
	return m_CompressCache;
}

void GOrgueSettings::SetCompressCache(bool compress)
{
	m_CompressCache = compress;
}

bool GOrgueSettings::GetScaleRelease()
{
	return m_ScaleRelease;
}

void GOrgueSettings::SetScaleRelease(bool scale_release)
{
	m_ScaleRelease = scale_release;
}

bool GOrgueSettings::GetLoadLastFile()
{
	return m_LoadLastFile;
}

void GOrgueSettings::SetLoadLastFile(bool last_file)
{
	m_LoadLastFile = last_file;
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
}

bool GOrgueSettings::GetRandomizeSpeaking()
{
	return m_RandomizeSpeaking;
}

void GOrgueSettings::SetRandomizeSpeaking(bool randomize)
{
	m_RandomizeSpeaking = randomize;
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
		bits_per_sample = 24;
	m_BitsPerSample = bits_per_sample;
	m_Config.Write(wxT("BitsPerSample"), (long)m_BitsPerSample);
}


unsigned GOrgueSettings::GetWaveFormatBytesPerSample()
{
	return m_WaveFormat;
}

void GOrgueSettings::SetWaveFormatBytesPerSample(unsigned bytes_per_sample)
{
	if (bytes_per_sample < 1)
		bytes_per_sample = 1;
	if (bytes_per_sample > 4)
		bytes_per_sample = 4;
	m_WaveFormat = bytes_per_sample;
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
}

int GOrgueSettings::GetVolume()
{
	return m_Volume;
}

void GOrgueSettings::SetVolume(int volume)
{
	if (volume > 20)
		volume = -15;
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

unsigned GOrgueSettings::GetAudioDeviceLatency(wxString device)
{
	return m_Config.Read(wxT("Devices/Sound/") + device, 60L);
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

ptr_vector<GOrgueOrgan>& GOrgueSettings::GetOrganList()
{
	return m_OrganList;
}

static bool LRUCompare(GOrgueOrgan* a, GOrgueOrgan* b)
{
	return a->GetLastUse() > b->GetLastUse();
}

std::vector<GOrgueOrgan*> GOrgueSettings::GetLRUOrganList()
{
	std::vector<GOrgueOrgan*> lru;
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		lru.push_back(m_OrganList[i]);
	std::sort(lru.begin(), lru.end(), LRUCompare);
	return lru;
}

void GOrgueSettings::AddOrgan(GOrgueOrgan* organ)
{
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		if (organ->GetODFPath() == m_OrganList[i]->GetODFPath())
		{
			m_OrganList[i]->Update(organ);
			delete organ;
			return;
		}
	m_OrganList.push_back(organ);
}

int GOrgueSettings::GetMidiInDeviceChannelShift(wxString device)
{
	std::map<wxString, int>::iterator it = m_MidiIn.find(device);
	if (it == m_MidiIn.end())
		return 0;
	else
		return it->second;
}

void GOrgueSettings::SetMidiInDeviceChannelShift(wxString device, int shift)
{
	m_Config.Write(wxT("Devices/MIDI/") + device, shift);
	m_MidiIn[device] = shift;
}

std::vector<wxString> GOrgueSettings::GetMidiInDeviceList()
{
	std::vector<wxString> list;
	for (std::map<wxString, int>::iterator it = m_MidiIn.begin(); it != m_MidiIn.end(); it++)
		list.push_back(it->first);
	return list;
}

int GOrgueSettings::GetMidiOutState(wxString device)
{
	std::map<wxString, bool>::iterator it = m_MidiOut.find(device);
	if (it == m_MidiOut.end())
		return false;
	else
		return it->second;
}

void GOrgueSettings::SetMidiOutState(wxString device, bool enabled)
{
	m_Config.Write(wxT("Devices/MIDIOut/") + device, enabled ? 1 : -1);
	m_MidiOut[device] = enabled;
}

std::vector<wxString> GOrgueSettings::GetMidiOutDeviceList()
{
	std::vector<wxString> list;
	for (std::map<wxString, bool>::iterator it = m_MidiOut.begin(); it != m_MidiOut.end(); it++)
		list.push_back(it->first);
	return list;
}

const std::vector<wxString>& GOrgueSettings::GetAudioGroups()
{
	return m_AudioGroups;
}

void GOrgueSettings::SetAudioGroups(const std::vector<wxString>& audio_groups)
{
	if (!audio_groups.size())
		return;
	m_AudioGroups = audio_groups;
}

unsigned GOrgueSettings::GetAudioGroupId(const wxString& str)
{
	for(unsigned i = 0; i < m_AudioGroups.size(); i++)
		if (m_AudioGroups[i] == str)
			return i;
	return 0;
}

int GOrgueSettings::GetStrictAudioGroupId(const wxString& str)
{
	for(unsigned i = 0; i < m_AudioGroups.size(); i++)
		if (m_AudioGroups[i] == str)
			return i;
	return -1;
}

const std::vector<GOAudioDeviceConfig>& GOrgueSettings::GetAudioDeviceConfig()
{
	return m_AudioDeviceConfig;
}

void GOrgueSettings::SetAudioDeviceConfig(const std::vector<GOAudioDeviceConfig>& config)
{
	if (!config.size())
		return;
	m_AudioDeviceConfig = config;
}

int GOrgueSettings::GetTranspose()
{
	return m_Transpose;
}

void GOrgueSettings::SetTranspose(int transpose)
{
	m_Transpose = transpose;
}

int GOrgueSettings::GetReverb()
{
	return m_Reverb;
}

void GOrgueSettings::SetReverb(int reverb)
{
	m_Reverb = reverb;
	m_Config.Write(wxT("Reverb"), reverb);
}

bool GOrgueSettings::GetReverbEnabled()
{
	return m_ReverbEnabled;
}

void GOrgueSettings::SetReverbEnabled(bool on)
{
	m_ReverbEnabled = on;
}

bool GOrgueSettings::GetReverbDirect()
{
	return m_ReverbDirect;
}

void GOrgueSettings::SetReverbDirect(bool on)
{
	m_ReverbDirect = on;
}

wxString GOrgueSettings::GetReverbFile()
{
	return m_ReverbFile;
}

void GOrgueSettings::SetReverbFile(wxString file)
{
	m_ReverbFile = file;
}

unsigned GOrgueSettings::GetReverbStartOffset()
{
	return m_ReverbStartOffset;
}

void GOrgueSettings::SetReverbStartOffset(unsigned offset)
{
	if (offset > 158760000)
		offset = 158760000;
	m_ReverbStartOffset = offset;
}

unsigned GOrgueSettings::GetReverbLen()
{
	return m_ReverbLen;
}

void GOrgueSettings::SetReverbLen(unsigned length)
{
	if (length > 158760000)
		length = 158760000;
	m_ReverbLen = length;
}

float GOrgueSettings::GetReverbGain()
{
	return m_ReverbGain;
}

void GOrgueSettings::SetReverbGain(float gain)
{
	if (gain <= 0)
		gain = 1;
	if (gain > 50)
		gain = 50;
	m_ReverbGain = gain;
}

int GOrgueSettings::GetReverbChannel()
{
	return m_ReverbChannel;
}

void GOrgueSettings::SetReverbChannel(int channel)
{
	if (channel < 1 || channel > 4)
		channel = 1;
	m_ReverbChannel = channel;
}

unsigned GOrgueSettings::GetReverbDelay()
{
	return m_ReverbDelay;
}

void GOrgueSettings::SetReverbDelay(unsigned delay)
{
	if (delay > 10000)
		delay = 10000;
	m_ReverbDelay = delay;
}

void GOrgueSettings::Flush()
{
	wxString tmp_name = m_ConfigFileName + wxT(".new");
	GOrgueConfigFileWriter cfg_file;
	GOrgueConfigWriter cfg(cfg_file, false);

	cfg.WriteInteger(wxT("General"), wxT("InterpolationType"), m_InterpolationType);
	cfg.WriteInteger(wxT("General"), wxT("WaveFormat"), m_WaveFormat);
	cfg.WriteInteger(wxT("General"), wxT("AttackLoad"), m_AttackLoad);
	cfg.WriteInteger(wxT("General"), wxT("LoopLoad"), m_LoopLoad);
	cfg.WriteInteger(wxT("General"), wxT("ReleaseLoad"), m_ReleaseLoad);

	cfg.WriteInteger(wxT("General"), wxT("Concurrency"), m_Concurrency);
	cfg.WriteInteger(wxT("General"), wxT("ReleaseConcurrency"), m_ReleaseConcurrency);
	cfg.WriteInteger(wxT("General"), wxT("LoadConcurrency"), m_LoadConcurrency);

	cfg.WriteBoolean(wxT("General"), wxT("ManageCache"), m_ManageCache);
	cfg.WriteBoolean(wxT("General"), wxT("CompressCache"), m_CompressCache);
	cfg.WriteBoolean(wxT("General"), wxT("LoadLastFile"), m_LoadLastFile);
	cfg.WriteString(wxT("General"), wxT("LastFile"), m_LastFile);

	cfg.WriteBoolean(wxT("General"), wxT("Stereo"), m_Stereo);
	cfg.WriteBoolean(wxT("General"), wxT("LosslessCompression"), m_LosslessCompression);
	cfg.WriteBoolean(wxT("General"), wxT("ManagePolyphony"), m_ManagePolyphony);
	cfg.WriteBoolean(wxT("General"), wxT("ScaleRelease"), m_ScaleRelease);
	cfg.WriteBoolean(wxT("General"), wxT("RandomizeSpeaking"), m_RandomizeSpeaking);
	cfg.WriteFloat(wxT("General"), wxT("MemoryLimit"), (double)m_MemoryLimit / (1024.0 * 1024.0));

	cfg.WriteInteger(wxT("General"), wxT("OrganCount"), m_OrganList.size());
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		m_OrganList[i]->Save(cfg, wxString::Format(wxT("Organ%03d"), i + 1));

	for(unsigned i = 0; i < GetEventCount(); i++)
		m_MIDIEvents[i]->Save(cfg, GetEventSection(i));

	for(unsigned i = 0; i < m_AudioGroups.size(); i++)
		cfg.WriteString(wxT("AudioGroups"), wxString::Format(wxT("Name%03d"), i + 1), m_AudioGroups[i]);
	cfg.WriteInteger(wxT("AudioGroups"), wxT("Count"), m_AudioGroups.size());

	for(unsigned i = 0; i < m_AudioDeviceConfig.size(); i++)
	{
		cfg.WriteString(wxT("AudioDevices"), wxString::Format(wxT("Device%03dName"), i + 1), m_AudioDeviceConfig[i].name);
		cfg.WriteInteger(wxT("AudioDevices"), wxString::Format(wxT("Device%03dChannelCount"), i + 1), m_AudioDeviceConfig[i].channels);
		for(unsigned j = 0; j < m_AudioDeviceConfig[i].channels; j++)
		{
			wxString prefix = wxString::Format(wxT("Device%03dChannel%03d"), i + 1, j + 1);
			cfg.WriteInteger(wxT("AudioDevices"), prefix + wxT("GroupCount"), m_AudioDeviceConfig[i].scale_factors[j].size());
			for(unsigned k = 0; k < m_AudioDeviceConfig[i].scale_factors[j].size(); k++)
			{
				wxString p = prefix + wxString::Format(wxT("Group%03d"), k + 1);
				cfg.WriteString(wxT("AudioDevices"), p + wxT("Name"), m_AudioDeviceConfig[i].scale_factors[j][k].name);
				cfg.WriteFloat(wxT("AudioDevices"), p + wxT("Left"), m_AudioDeviceConfig[i].scale_factors[j][k].left);
				cfg.WriteFloat(wxT("AudioDevices"), p + wxT("Right"), m_AudioDeviceConfig[i].scale_factors[j][k].right);
			}
		}
	}
	cfg.WriteInteger(wxT("AudioDevices"), wxT("Count"), m_AudioDeviceConfig.size());

	cfg.WriteBoolean(wxT("Reverb"), wxT("ReverbEnabled"), m_ReverbEnabled);
	cfg.WriteBoolean(wxT("Reverb"), wxT("ReverbDirect"), m_ReverbDirect);
	cfg.WriteInteger(wxT("Reverb"), wxT("ReverbChannel"), m_ReverbChannel);
	cfg.WriteInteger(wxT("Reverb"), wxT("ReverbStartOffset"), m_ReverbStartOffset);
	cfg.WriteInteger(wxT("Reverb"), wxT("ReverbLen"), m_ReverbLen);
	cfg.WriteInteger(wxT("Reverb"), wxT("ReverbDelay"), m_ReverbDelay);
	cfg.WriteFloat(wxT("Reverb"), wxT("ReverbGain"), m_ReverbGain);
	cfg.WriteString(wxT("Reverb"), wxT("ReverbFile"), m_ReverbFile);

	if (::wxFileExists(tmp_name) && !::wxRemoveFile(tmp_name))
	{
		wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
		return;
	}
	if (!cfg_file.Save(tmp_name))
	{
		wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
		return;
	}
	if (!GORenameFile(tmp_name, m_ConfigFileName))
		return;

	m_Config.Flush();
}
