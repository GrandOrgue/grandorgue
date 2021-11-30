/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSETTINGS_H
#define GOSETTINGS_H

#include <map>
#include <vector>
#include <wx/gdicmn.h>
#include <wx/string.h>

#include "midi/GOMidiMap.h"
#include "midi/GOMidiReceiverBase.h"
#include "settings/GOSettingBool.h"
#include "settings/GOSettingDirectory.h"
#include "settings/GOSettingEnum.h"
#include "settings/GOSettingFile.h"
#include "settings/GOSettingFloat.h"
#include "settings/GOSettingNumber.h"
#include "settings/GOSettingString.h"
#include "settings/GOSettingStore.h"
#include "settings/GOSettingString.h"
#include "temperaments/GOTemperamentList.h"
#include "GOOrganList.h"
#include "GOPortsConfig.h"
#include "ptrvector.h"

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

enum class GOInitialLoadType {
	LOAD_NONE,
	LOAD_LAST_USED,
	LOAD_FIRST
};

class GOSettings : public GOSettingStore, public GOOrganList {
private:
	wxString m_InstanceName;
	wxString m_ConfigFileName;
	std::map<wxString, bool> m_MidiIn;
	std::map<wxString, unsigned> m_MidiInShift;
	std::map<wxString, wxString> m_MidiInOutDeviceMap;
	std::map<wxString, bool> m_MidiOut;
	wxString m_ResourceDir;
	std::vector<wxString> m_AudioGroups;
	GOPortsConfig m_SoundPortsConfig;
	std::vector<GOAudioDeviceConfig> m_AudioDeviceConfig;
	ptr_vector<GOMidiReceiverBase> m_MIDIEvents;
	GOMidiMap m_MidiMap;
	GOTemperamentList m_Temperaments;
	int m_MainWindowX;
	int m_MainWindowY;
	unsigned m_MainWindowWidth;
	unsigned m_MainWindowHeight;
	
	static const GOMidiSetting m_MIDISettings[];
	static const struct IniFileEnumEntry m_InitialLoadTypes[];

	wxString GetEventSection(unsigned index);

public:
	GOSettings(wxString instance);
	~GOSettings();

	GOSettingDirectory UserSettingPath;
	GOSettingDirectory UserCachePath;

	GOSettingUnsigned Concurrency;
	GOSettingUnsigned ReleaseConcurrency;
	GOSettingUnsigned LoadConcurrency;

	GOSettingUnsigned InterpolationType;
	GOSettingUnsigned WaveFormatBytesPerSample;
	GOSettingBool RecordDownmix;

	GOSettingUnsigned AttackLoad;
	GOSettingUnsigned LoopLoad;
	GOSettingUnsigned ReleaseLoad;

	GOSettingBool ManageCache;
	GOSettingBool CompressCache;
	GOSettingEnum<GOInitialLoadType> LoadLastFile;
	GOSettingBool ODFCheck;

	GOSettingUnsigned LoadChannels;
	GOSettingBool LosslessCompression;
	GOSettingBool ManagePolyphony;
	GOSettingBool ScaleRelease;
	GOSettingBool RandomizeSpeaking;
	GOSettingBool ReverbEnabled;
	GOSettingBool ReverbDirect;
	GOSettingUnsigned ReverbChannel;
	GOSettingUnsigned ReverbStartOffset;
	GOSettingUnsigned ReverbLen;
	GOSettingUnsigned ReverbDelay;
	GOSettingFloat ReverbGain;
	GOSettingFile ReverbFile;

	GOSettingFloat MemoryLimit;
	GOSettingUnsigned SamplesPerBuffer;
	GOSettingUnsigned SampleRate;
	GOSettingInteger Volume;
	GOSettingUnsigned PolyphonyLimit;
	GOSettingUnsigned Preset;
	GOSettingUnsigned ReleaseLength;
	GOSettingString LanguageCode;

	class GOSettingUnsignedBit : public GOSettingUnsigned
	{
	protected:
		unsigned validate(unsigned value)
		{
			return GOSettingUnsigned::validate(value - value % 4);
		}

	public:
		GOSettingUnsignedBit(GOSettingStore* store, wxString group, wxString name, unsigned min_value, unsigned max_value, unsigned default_value) :
			GOSettingUnsigned(store, group, name, min_value, max_value, default_value)
		{
		}

	} BitsPerSample;
	GOSettingInteger Transpose;

	GOSettingUnsigned MetronomeMeasure;
	GOSettingUnsigned MetronomeBPM;

	GOSettingString MidiRecorderOutputDevice;

	GOSettingDirectory OrganPath;
	GOSettingDirectory OrganPackagePath;
	GOSettingDirectory SettingPath;
	GOSettingDirectory AudioRecorderPath;
	GOSettingDirectory MidiRecorderPath;
	GOSettingDirectory MidiPlayerPath;
	
	void Load();

	int GetLanguageId() const;
	void SetLanguageId(int langId);
	
	const wxString GetResourceDirectory();
	const wxString GetPackageDirectory();

	unsigned GetEventCount();
	wxString GetEventGroup(unsigned index);
	wxString GetEventTitle(unsigned index);
	GOMidiReceiverBase* GetMidiEvent(unsigned index);
	GOMidiReceiverBase* FindMidiEvent(MIDI_RECEIVER_TYPE type, unsigned index);
	
	bool GetMidiInState(wxString device);
	void SetMidiInState(wxString device, bool enabled);
	unsigned GetMidiInDeviceChannelShift(wxString device);
	void SetMidiInDeviceChannelShift(wxString device, unsigned shift);
	wxString GetMidiInOutDevice(wxString device);
	void SetMidiInOutDevice(wxString device, wxString out_device);
	std::vector<wxString> GetMidiInDeviceList();

	bool GetMidiOutState(wxString device);
	void SetMidiOutState(wxString device, bool enabled);
	std::vector<wxString> GetMidiOutDeviceList();

	const std::vector<wxString>& GetAudioGroups();
	void SetAudioGroups(const std::vector<wxString>& audio_groups);
	unsigned GetAudioGroupId(const wxString& str);
	int GetStrictAudioGroupId(const wxString& str);

	const GOPortsConfig & GetSoundPortsConfig() const
	{ return m_SoundPortsConfig; }
	
	void SetSoundPortsConfig(const GOPortsConfig & portsConfig)
	{ m_SoundPortsConfig = portsConfig; }
	
	const std::vector<GOAudioDeviceConfig>& GetAudioDeviceConfig();
	const unsigned GetTotalAudioChannels() const;
	void SetAudioDeviceConfig(const std::vector<GOAudioDeviceConfig>& config);
	unsigned GetDefaultLatency();

	GOMidiMap& GetMidiMap();

	GOTemperamentList& GetTemperaments();
	
	wxRect GetMainWindowRect();
	void SetMainWindowRect(const wxRect &rect);

	void Flush();
};

#endif
