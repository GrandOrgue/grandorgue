/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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
#include "GOrgueArchiveFile.h"
#include "GOrgueConfigFileWriter.h"
#include "GOrgueConfigFileReader.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigReaderDB.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueLimits.h"
#include "GOrgueMemoryPool.h"
#include "GOrgueOrgan.h"
#include "GOrguePath.h"
#include "GOrgueSettingNumber.cpp"
#include "GOrgueStdPath.h"
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
	{ MIDI_RECV_SETTER, 25, wxTRANSLATE("Metronome"), wxTRANSLATE("On") },
	{ MIDI_RECV_SETTER, 26, wxTRANSLATE("Metronome"), wxTRANSLATE("BPM +") },
	{ MIDI_RECV_SETTER, 27, wxTRANSLATE("Metronome"), wxTRANSLATE("BPM -") },
	{ MIDI_RECV_SETTER, 28, wxTRANSLATE("Metronome"), wxTRANSLATE("Measure -") },
	{ MIDI_RECV_SETTER, 29, wxTRANSLATE("Metronome"), wxTRANSLATE("Measure +") },
};

GOrgueSettings::GOrgueSettings(wxString instance) :
	m_InstanceName(instance),
	m_OrganList(),
	m_ArchiveList(),
	m_ResourceDir(),
	m_AudioGroups(),
	m_AudioDeviceConfig(),
	m_MIDIEvents(),
	UserSettingPath(this, wxT("General"), wxT("SettingPath"), wxEmptyString),
	UserCachePath(this, wxT("General"), wxT("CachePath"), wxEmptyString),
	Concurrency(this, wxT("General"), wxT("Concurrency"), 0, MAX_CPU, 1),
	ReleaseConcurrency(this, wxT("General"), wxT("ReleaseConcurrency"), 1, MAX_CPU, 1),
	LoadConcurrency(this, wxT("General"), wxT("LoadConcurrency"), 0, MAX_CPU, 1),
	InterpolationType(this, wxT("General"), wxT("InterpolationType"), 0, 1, 0),
	WaveFormatBytesPerSample(this, wxT("General"), wxT("WaveFormat"), 1, 4, 4),
	RecordDownmix(this, wxT("General"), wxT("RecordDownmix"), false),
	AttackLoad(this, wxT("General"), wxT("AttackLoad"), 0, 1, 1),
	LoopLoad(this, wxT("General"), wxT("LoopLoad"), 0, 2, 2),
	ReleaseLoad(this, wxT("General"), wxT("ReleaseLoad"), 0, 1, 1),
	ManageCache(this, wxT("General"), wxT("ManageCache"), true),
	CompressCache(this, wxT("General"), wxT("CompressCache"), false),
	LoadLastFile(this, wxT("General"), wxT("LoadLastFile"), true),
	ODFCheck(this, wxT("General"), wxT("StrictODFCheck"), false),
	LoadChannels(this, wxT("General"), wxT("Channels"), 0, 2, 2),
	LosslessCompression(this, wxT("General"), wxT("LosslessCompression"), false),
	ManagePolyphony(this, wxT("General"), wxT("ManagePolyphony"), true),
	ScaleRelease(this, wxT("General"), wxT("ScaleRelease"), true),
	RandomizeSpeaking(this, wxT("General"), wxT("RandomizeSpeaking"), true),
	ReverbEnabled(this, wxT("Reverb"), wxT("ReverbEnabled"), false),
	ReverbDirect(this, wxT("Reverb"), wxT("ReverbDirect"), true),
	ReverbChannel(this, wxT("Reverb"), wxT("ReverbChannel"), 1, 4, 1),
	ReverbStartOffset(this, wxT("Reverb"), wxT("ReverbStartOffset"), 0, MAX_SAMPLE_LENGTH, 0),
	ReverbLen(this, wxT("Reverb"), wxT("ReverbLen"), 0, MAX_SAMPLE_LENGTH, 0),
	ReverbDelay(this, wxT("Reverb"), wxT("ReverbDelay"), 0, 10000, 0),
	ReverbGain(this, wxT("Reverb"), wxT("ReverbGain"), 0, 50, 1),
	ReverbFile(this, wxT("Reverb"), wxT("ReverbFile"), wxEmptyString),
	MemoryLimit(this, wxT("General"), wxT("MemoryLimit"), 0, 1024 * 1024, GOrgueMemoryPool::GetSystemMemoryLimit()),
	SamplesPerBuffer(this, wxT("General"), wxT("SamplesPerBuffer"), 1, MAX_FRAME_SIZE, 1024),
	SampleRate(this, wxT("General"), wxT("SampleRate"), 1000, 100000, 44100),
	Volume(this, wxT("General"), wxT("Volume"), -120, 20, -15),
	PolyphonyLimit(this, wxT("General"), wxT("PolyphonyLimit"), 0, MAX_POLYPHONY, 2048),
	Preset(this, wxT("General"), wxT("Preset"), 0, MAX_PRESET, 0),
	ReleaseLength(this, wxT("General"), wxT("ReleaseLength"), 0, 3000, 0),
	BitsPerSample(this, wxT("General"), wxT("BitsPerSample"), 8, 24, 24),
	Transpose(this, wxT("General"), wxT("Transpose"), -11, 11, 0),
	MetronomeMeasure(this, wxT("Metronome"), wxT("Measure"), 0, 32, 4),
	MetronomeBPM(this, wxT("Metronome"), wxT("BPM"), 1, 500, 80),
	MidiRecorderOutputDevice(this, wxT("MIDIOut"), wxT("MIDIRecorderDevice"), wxEmptyString),
	OrganPath(this, wxT("General"), wxT("OrganPath"), wxEmptyString),
	OrganPackagePath(this, wxT("General"), wxT("OrganPackagePath"), wxEmptyString),
	SettingPath(this, wxT("General"), wxT("CMBPath"), wxEmptyString),
	AudioRecorderPath(this, wxT("General"), wxT("AudioRecorder"), wxEmptyString),
	MidiRecorderPath(this, wxT("General"), wxT("MIDIRecorderPath"), wxEmptyString),
	MidiPlayerPath(this, wxT("General"), wxT("MIDIPlayerPath"), wxEmptyString)
{
	m_ConfigFileName = wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueConfig") + m_InstanceName;
	for(unsigned i = 0; i < GetEventCount(); i++)
		m_MIDIEvents.push_back(new GOrgueMidiReceiver(NULL, m_MIDISettings[i].type));
	m_ResourceDir = GOrgueStdPath::GetResourceDir();

	UserSettingPath.setDefaultValue(wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueData") + m_InstanceName);
	UserCachePath.setDefaultValue(wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() + wxT("GrandOrgueCache") + m_InstanceName);

	wxString docdir = wxStandardPaths::Get().GetDocumentsDir() + wxFileName::GetPathSeparator() + _("GrandOrgue") + wxFileName::GetPathSeparator();
	OrganPath.setDefaultValue(docdir + _("Organs"));
	OrganPackagePath.setDefaultValue(docdir + _("Organ packages"));
	SettingPath.setDefaultValue(docdir + _("Settings"));
	AudioRecorderPath.setDefaultValue(docdir + _("Audio recordings"));
	MidiRecorderPath.setDefaultValue(docdir + _("MIDI recordings"));
	MidiPlayerPath.setDefaultValue(docdir + _("MIDI recordings"));
}

GOrgueSettings::~GOrgueSettings()
{
	Flush();
}

void GOrgueSettings::Load()
{
	GOrgueConfigFileReader cfg_file;
	if (wxFileExists(m_ConfigFileName))
	{
		if (!cfg_file.Read(m_ConfigFileName))
			wxLogError(_("Unable to read '%s'"), m_ConfigFileName.c_str());
	}

	try
	{
		GOrgueConfigReaderDB cfg_db;
		cfg_db.ReadData(cfg_file, CMBSetting, false);
		GOrgueConfigReader cfg(cfg_db);

		m_OrganList.clear();
		unsigned organ_count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("OrganCount"), 0, 99999, false, 0);
		for(unsigned i = 0; i < organ_count; i++)
			m_OrganList.push_back(new GOrgueOrgan(cfg, wxString::Format(wxT("Organ%03d"), i + 1), m_MidiMap));

		m_ArchiveList.clear();
		unsigned archive_count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("ArchiveCount"), 0, 99999, false, 0);
		for(unsigned i = 0; i < archive_count; i++)
			m_ArchiveList.push_back(new GOrgueArchiveFile(cfg, wxString::Format(wxT("Archive%03d"), i + 1)));

		m_Temperaments.InitTemperaments();
		m_Temperaments.Load(cfg);

		m_AudioGroups.clear();
		unsigned count = cfg.ReadInteger(CMBSetting, wxT("AudioGroups"), wxT("Count"), 0, 200, false, 0);
		for(unsigned i = 0; i < count; i++)
			m_AudioGroups.push_back(cfg.ReadString(CMBSetting, wxT("AudioGroups"), wxString::Format(wxT("Name%03d"), i + 1), false, wxString::Format(_("Audio group %d"), i + 1)));
		if (!m_AudioGroups.size())
			m_AudioGroups.push_back(_("Default audio group"));
		
		m_AudioDeviceConfig.clear();
		count = cfg.ReadInteger(CMBSetting, wxT("AudioDevices"), wxT("Count"), 0, 200, false, 0);
		for(unsigned i = 0; i < count; i++)
		{
			GOAudioDeviceConfig conf;
			conf.name = cfg.ReadString(CMBSetting, wxT("AudioDevices"), wxString::Format(wxT("Device%03dName"), i + 1));
			conf.channels = cfg.ReadInteger(CMBSetting, wxT("AudioDevices"), wxString::Format(wxT("Device%03dChannelCount"), i + 1), 0, 200);
			conf.desired_latency = cfg.ReadInteger(CMBSetting, wxT("AudioDevices"), wxString::Format(wxT("Device%03dLatency"), i + 1), 0, 999, false, GetDefaultLatency());
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

		for(unsigned i = 0; i < GetEventCount(); i++)
			m_MIDIEvents[i]->Load(cfg, GetEventSection(i), m_MidiMap);

		long cpus = wxThread::GetCPUCount();
		if (cpus == -1)
			cpus = 4;
		if (cpus > MAX_CPU)
			cpus = MAX_CPU;
		if (cpus == 0)
			cpus = 1;

		Concurrency.setDefaultValue(cpus);
		ReleaseConcurrency.setDefaultValue(cpus);
		LoadConcurrency.setDefaultValue(cpus);
		PolyphonyLimit.setDefaultValue(cpus * 725),

		GOrgueSettingStore::Load(cfg);

		if (Concurrency() == 0)
			Concurrency(1);

		count = cfg.ReadInteger(CMBSetting, wxT("MIDIIn"), wxT("Count"), 0, MAX_MIDI_DEVICES, false, 0);
		for(unsigned i = 0; i < count; i++)
		{
			wxString name = cfg.ReadString(CMBSetting, wxT("MIDIIn"), wxString::Format(wxT("Device%03d"), i + 1));
			SetMidiInState(name, cfg.ReadBoolean(CMBSetting, wxT("MIDIIn"), wxString::Format(wxT("Device%03dEnabled"), i + 1)));
			SetMidiInDeviceChannelShift(name, cfg.ReadInteger(CMBSetting, wxT("MIDIIn"), wxString::Format(wxT("Device%03dShift"), i + 1), 0, 15));;
		}

		count = cfg.ReadInteger(CMBSetting, wxT("MIDIOut"), wxT("Count"), 0, MAX_MIDI_DEVICES, false, 0);
		for(unsigned i = 0; i < count; i++)
		{
			wxString name = cfg.ReadString(CMBSetting, wxT("MIDIOut"), wxString::Format(wxT("Device%03d"), i + 1));
			SetMidiOutState(name, cfg.ReadBoolean(CMBSetting, wxT("MIDIOut"), wxString::Format(wxT("Device%03dEnabled"), i + 1)));
		}

		if (wxFileExists(m_ConfigFileName))
			wxCopyFile(m_ConfigFileName, m_ConfigFileName + wxT(".last"));
	}
	catch (wxString error)
	{
		wxLogError(wxT("%s\n"),error.c_str());
	}

	if (!m_AudioDeviceConfig.size())
	{
		GOAudioDeviceConfig conf;
		conf.name = wxEmptyString;
		conf.channels = 2;
		conf.scale_factors.resize(conf.channels);
		conf.desired_latency = GetDefaultLatency();
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
		return wxString::Format(wxT("Enclosure%03d"), m_MIDISettings[index].index);

	case MIDI_RECV_MANUAL:
		return wxString::Format(wxT("Manual%03d"), m_MIDISettings[index].index);

	case MIDI_RECV_SETTER:
		return wxString::Format(wxT("Setter%03d"), m_MIDISettings[index].index);
		
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

const wxString GOrgueSettings::GetResourceDirectory()
{
	return m_ResourceDir.c_str();
}

const wxString GOrgueSettings::GetPackageDirectory()
{
	return m_ResourceDir + wxFileName::GetPathSeparator() + wxT("packages");
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
		if (m_OrganList[i]->IsUsable(*this))
			lru.push_back(m_OrganList[i]);
	std::sort(lru.begin(), lru.end(), LRUCompare);
	return lru;
}

void GOrgueSettings::AddOrgan(const GOrgueOrgan& organ)
{
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		if (organ.GetODFPath() == m_OrganList[i]->GetODFPath() &&
		    organ.GetArchiveID() == m_OrganList[i]->GetArchiveID())
		{
			m_OrganList[i]->Update(organ);
			return;
		}
	m_OrganList.push_back(new GOrgueOrgan(organ));
}

ptr_vector<GOrgueArchiveFile>& GOrgueSettings::GetArchiveList()
{
	return m_ArchiveList;
}

GOrgueArchiveFile* GOrgueSettings::GetArchiveByID(const wxString& id, bool useable)
{
	for(unsigned i = 0; i < m_ArchiveList.size(); i++)
		if (m_ArchiveList[i]->GetID() == id)
			if (!useable || m_ArchiveList[i]->IsUsable(*this))
				return m_ArchiveList[i];
	return NULL;
}

GOrgueArchiveFile* GOrgueSettings::GetArchiveByPath(const wxString& path)
{
	for(unsigned i = 0; i < m_ArchiveList.size(); i++)
		if (m_ArchiveList[i]->GetPath() == path)
			return m_ArchiveList[i];
	return NULL;
}

void GOrgueSettings::AddArchive(const GOrgueArchiveFile& archive)
{
	GOrgueArchiveFile* a = GetArchiveByPath(archive.GetPath());
	if (a)
		a->Update(archive);
	else
		m_ArchiveList.push_back(new GOrgueArchiveFile(archive));
}

bool GOrgueSettings::GetMidiInState(wxString device)
{
	std::map<wxString, bool>::iterator it = m_MidiIn.find(device);
	if (it == m_MidiIn.end())
	{
		m_MidiIn[device] = device.Find(wxT("GrandOrgue")) == wxNOT_FOUND;
		return m_MidiIn[device];
	}
	else
		return it->second;
}

void GOrgueSettings::SetMidiInState(wxString device, bool enabled)
{
	m_MidiIn[device] = enabled;
}

unsigned GOrgueSettings::GetMidiInDeviceChannelShift(wxString device)
{
	std::map<wxString, unsigned>::iterator it = m_MidiInShift.find(device);
	if (it == m_MidiInShift.end())
		return 0;
	else
		return it->second;
}

void GOrgueSettings::SetMidiInDeviceChannelShift(wxString device, unsigned shift)
{
	shift = shift % 16;
	m_MidiInShift[device] = shift;
}

std::vector<wxString> GOrgueSettings::GetMidiInDeviceList()
{
	std::vector<wxString> list;
	for (std::map<wxString, bool>::iterator it = m_MidiIn.begin(); it != m_MidiIn.end(); it++)
		list.push_back(it->first);
	return list;
}

bool GOrgueSettings::GetMidiOutState(wxString device)
{
	std::map<wxString, bool>::iterator it = m_MidiOut.find(device);
	if (it == m_MidiOut.end())
		return false;
	else
		return it->second;
}

void GOrgueSettings::SetMidiOutState(wxString device, bool enabled)
{
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

unsigned GOrgueSettings::GetDefaultLatency()
{
	return 50;
}

GOrgueMidiMap& GOrgueSettings::GetMidiMap()
{
	return m_MidiMap;
}

GOrgueTemperamentList& GOrgueSettings::GetTemperaments()
{
	return m_Temperaments;
}

void GOrgueSettings::Flush()
{
	wxString tmp_name = m_ConfigFileName + wxT(".new");
	GOrgueConfigFileWriter cfg_file;
	GOrgueConfigWriter cfg(cfg_file, false);

	GOrgueSettingStore::Save(cfg);

	cfg.WriteInteger(wxT("General"), wxT("ArchiveCount"), m_ArchiveList.size());
	for(unsigned i = 0; i < m_ArchiveList.size(); i++)
		m_ArchiveList[i]->Save(cfg, wxString::Format(wxT("Archive%03d"), i + 1));

	cfg.WriteInteger(wxT("General"), wxT("OrganCount"), m_OrganList.size());
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		m_OrganList[i]->Save(cfg, wxString::Format(wxT("Organ%03d"), i + 1), m_MidiMap);

	m_Temperaments.Save(cfg);

	for(unsigned i = 0; i < GetEventCount(); i++)
		m_MIDIEvents[i]->Save(cfg, GetEventSection(i), m_MidiMap);

	for(unsigned i = 0; i < m_AudioGroups.size(); i++)
		cfg.WriteString(wxT("AudioGroups"), wxString::Format(wxT("Name%03d"), i + 1), m_AudioGroups[i]);
	cfg.WriteInteger(wxT("AudioGroups"), wxT("Count"), m_AudioGroups.size());

	for(unsigned i = 0; i < m_AudioDeviceConfig.size(); i++)
	{
		cfg.WriteString(wxT("AudioDevices"), wxString::Format(wxT("Device%03dName"), i + 1), m_AudioDeviceConfig[i].name);
		cfg.WriteInteger(wxT("AudioDevices"), wxString::Format(wxT("Device%03dChannelCount"), i + 1), m_AudioDeviceConfig[i].channels);
		cfg.WriteInteger(wxT("AudioDevices"), wxString::Format(wxT("Device%03dLatency"), i + 1), m_AudioDeviceConfig[i].desired_latency);
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

	unsigned count = 0;
	for (std::map<wxString, bool>::iterator it = m_MidiIn.begin(); it != m_MidiIn.end(); it++)
	{
		count++;
		cfg.WriteString(wxT("MIDIIn"), wxString::Format(wxT("Device%03d"), count), it->first);
		cfg.WriteBoolean(wxT("MIDIIn"), wxString::Format(wxT("Device%03dEnabled"), count), it->second);
		cfg.WriteInteger(wxT("MIDIIn"), wxString::Format(wxT("Device%03dShift"), count), GetMidiInDeviceChannelShift(it->first));
	}
	if (count > MAX_MIDI_DEVICES)
		count = MAX_MIDI_DEVICES;
	cfg.WriteInteger(wxT("MIDIIn"), wxT("Count"), count);

	count = 0;
	for (std::map<wxString, bool>::iterator it = m_MidiOut.begin(); it != m_MidiOut.end(); it++)
	{
		count++;
		cfg.WriteString(wxT("MIDIOut"), wxString::Format(wxT("Device%03d"), count), it->first);
		cfg.WriteBoolean(wxT("MIDIOut"), wxString::Format(wxT("Device%03dEnabled"), count), it->second);
	}
	if (count > MAX_MIDI_DEVICES)
		count = MAX_MIDI_DEVICES;
	cfg.WriteInteger(wxT("MIDIOut"), wxT("Count"), count);

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
}
