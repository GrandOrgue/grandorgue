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

#ifndef GORGUESETTINGS_H
#define GORGUESETTINGS_H

#include "GOrgueMidiMap.h"
#include "GOrgueMidiReceiver.h"
#include "GOrgueTemperamentList.h"
#include "GOrgueSettingBool.h"
#include "GOrgueSettingDirectory.h"
#include "GOrgueSettingFile.h"
#include "GOrgueSettingFloat.h"
#include "GOrgueSettingNumber.h"
#include "GOrgueSettingString.h"
#include "GOrgueSettingStore.h"
#include "GOrgueSettingString.h"
#include "ptrvector.h"
#include <wx/string.h>
#include <map>
#include <vector>

class GOrgueArchiveFile;
class GOrgueOrgan;

typedef struct
{
	wxString name;
	float left;
	float right;
} GOAudioGroupOutputConfig;

typedef struct
{
	wxString name;
	unsigned channels;
	unsigned desired_latency;
	std::vector< std::vector<GOAudioGroupOutputConfig> > scale_factors;
} GOAudioDeviceConfig;

typedef struct
{
	MIDI_RECEIVER_TYPE type;
	unsigned index;
	const wxString group;
	const wxString name;
} GOMidiSetting;

class GOrgueSettings : public GOrgueSettingStore {
private:
	wxString m_InstanceName;
	wxString m_ConfigFileName;
	ptr_vector<GOrgueOrgan> m_OrganList;
	ptr_vector<GOrgueArchiveFile> m_ArchiveList;
	std::map<wxString, bool> m_MidiIn;
	std::map<wxString, unsigned> m_MidiInShift;
	std::map<wxString, bool> m_MidiOut;
	wxString m_ResourceDir;
	std::vector<wxString> m_AudioGroups;
	std::vector<GOAudioDeviceConfig> m_AudioDeviceConfig;
	ptr_vector<GOrgueMidiReceiver> m_MIDIEvents;
	GOrgueMidiMap m_MidiMap;
	GOrgueTemperamentList m_Temperaments;

	static const GOMidiSetting m_MIDISettings[];

	wxString GetEventSection(unsigned index);

public:
	GOrgueSettings(wxString instance);
	~GOrgueSettings();

	GOrgueSettingDirectory UserSettingPath;
	GOrgueSettingDirectory UserCachePath;

	GOrgueSettingUnsigned Concurrency;
	GOrgueSettingUnsigned ReleaseConcurrency;
	GOrgueSettingUnsigned LoadConcurrency;

	GOrgueSettingUnsigned InterpolationType;
	GOrgueSettingUnsigned WaveFormatBytesPerSample;
	GOrgueSettingBool RecordDownmix;

	GOrgueSettingUnsigned AttackLoad;
	GOrgueSettingUnsigned LoopLoad;
	GOrgueSettingUnsigned ReleaseLoad;

	GOrgueSettingBool ManageCache;
	GOrgueSettingBool CompressCache;
	GOrgueSettingBool LoadLastFile;
	GOrgueSettingBool ODFCheck;

	GOrgueSettingUnsigned LoadChannels;
	GOrgueSettingBool LosslessCompression;
	GOrgueSettingBool ManagePolyphony;
	GOrgueSettingBool ScaleRelease;
	GOrgueSettingBool RandomizeSpeaking;
	GOrgueSettingBool ReverbEnabled;
	GOrgueSettingBool ReverbDirect;
	GOrgueSettingUnsigned ReverbChannel;
	GOrgueSettingUnsigned ReverbStartOffset;
	GOrgueSettingUnsigned ReverbLen;
	GOrgueSettingUnsigned ReverbDelay;
	GOrgueSettingFloat ReverbGain;
	GOrgueSettingFile ReverbFile;

	GOrgueSettingFloat MemoryLimit;
	GOrgueSettingUnsigned SamplesPerBuffer;
	GOrgueSettingUnsigned SampleRate;
	GOrgueSettingInteger Volume;
	GOrgueSettingUnsigned PolyphonyLimit;
	GOrgueSettingUnsigned Preset;
	GOrgueSettingUnsigned ReleaseLength;
	class GOrgueSettingUnsignedBit : public GOrgueSettingUnsigned
	{
	protected:
		unsigned validate(unsigned value)
		{
			return GOrgueSettingUnsigned::validate(value - value % 4);
		}

	public:
		GOrgueSettingUnsignedBit(GOrgueSettingStore* store, wxString group, wxString name, unsigned min_value, unsigned max_value, unsigned default_value) :
			GOrgueSettingUnsigned(store, group, name, min_value, max_value, default_value)
		{
		}

	} BitsPerSample;
	GOrgueSettingInteger Transpose;

	GOrgueSettingUnsigned MetronomeMeasure;
	GOrgueSettingUnsigned MetronomeBPM;

	GOrgueSettingString MidiRecorderOutputDevice;

	GOrgueSettingDirectory OrganPath;
	GOrgueSettingDirectory OrganPackagePath;
	GOrgueSettingDirectory SettingPath;
	GOrgueSettingDirectory AudioRecorderPath;
	GOrgueSettingDirectory MidiRecorderPath;
	GOrgueSettingDirectory MidiPlayerPath;

	void Load();
	const wxString GetResourceDirectory();
	const wxString GetPackageDirectory();

	unsigned GetEventCount();
	wxString GetEventGroup(unsigned index);
	wxString GetEventTitle(unsigned index);
	GOrgueMidiReceiver* GetMidiEvent(unsigned index);
	GOrgueMidiReceiver* FindMidiEvent(MIDI_RECEIVER_TYPE type, unsigned index);
	
	bool GetMidiInState(wxString device);
	void SetMidiInState(wxString device, bool enabled);
	unsigned GetMidiInDeviceChannelShift(wxString device);
	void SetMidiInDeviceChannelShift(wxString device, unsigned shift);
	std::vector<wxString> GetMidiInDeviceList();

	bool GetMidiOutState(wxString device);
	void SetMidiOutState(wxString device, bool enabled);
	std::vector<wxString> GetMidiOutDeviceList();

	const std::vector<wxString>& GetAudioGroups();
	void SetAudioGroups(const std::vector<wxString>& audio_groups);
	unsigned GetAudioGroupId(const wxString& str);
	int GetStrictAudioGroupId(const wxString& str);

	const std::vector<GOAudioDeviceConfig>& GetAudioDeviceConfig();
	void SetAudioDeviceConfig(const std::vector<GOAudioDeviceConfig>& config);
	unsigned GetDefaultLatency();

	void AddOrgan(const GOrgueOrgan& organ);
	ptr_vector<GOrgueOrgan>& GetOrganList();
	std::vector<GOrgueOrgan*> GetLRUOrganList();

	void AddArchive(const GOrgueArchiveFile& archive);
	ptr_vector<GOrgueArchiveFile>& GetArchiveList();
	GOrgueArchiveFile* GetArchiveByID(const wxString& id, bool useable = false);
	GOrgueArchiveFile* GetArchiveByPath(const wxString& path);

	GOrgueMidiMap& GetMidiMap();

	GOrgueTemperamentList& GetTemperaments();

	void Flush();
};

#endif
