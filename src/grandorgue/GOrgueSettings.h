/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTINGS_H
#define GORGUESETTINGS_H

#include "GOrgueMidiMap.h"
#include "GOrgueMidiReceiverBase.h"
#include "GOrgueOrganList.h"
#include "GOrgueTemperamentList.h"
#include "GOrgueSettingBool.h"
#include "GOrgueSettingDirectory.h"
#include "GOrgueSettingEnum.h"
#include "GOrgueSettingFile.h"
#include "GOrgueSettingFloat.h"
#include "GOrgueSettingNumber.h"
#include "GOrgueSettingString.h"
#include "GOrgueSettingStore.h"
#include "GOrgueSettingString.h"
#include "GOrgueSoundPortsConfig.h"
#include "ptrvector.h"
#include <wx/gdicmn.h>
#include <wx/string.h>
#include <map>
#include <vector>

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

class GOrgueSettings : public GOrgueSettingStore, public GOrgueOrganList {
private:
	wxString m_InstanceName;
	wxString m_ConfigFileName;
	std::map<wxString, bool> m_MidiIn;
	std::map<wxString, unsigned> m_MidiInShift;
	std::map<wxString, wxString> m_MidiInOutDeviceMap;
	std::map<wxString, bool> m_MidiOut;
	wxString m_ResourceDir;
	std::vector<wxString> m_AudioGroups;
	GOrgueSoundPortsConfig m_PortsConfig;
	std::vector<GOAudioDeviceConfig> m_AudioDeviceConfig;
	ptr_vector<GOrgueMidiReceiverBase> m_MIDIEvents;
	GOrgueMidiMap m_MidiMap;
	GOrgueTemperamentList m_Temperaments;
	int m_MainWindowX;
	int m_MainWindowY;
	unsigned m_MainWindowWidth;
	unsigned m_MainWindowHeight;

	static const GOMidiSetting m_MIDISettings[];
	static const struct IniFileEnumEntry m_InitialLoadTypes[];

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
	GOrgueSettingEnum<GOInitialLoadType> LoadLastFile;
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
	GOrgueSettingString LanguageCode;

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

	int GetLanguageId() const;
	void SetLanguageId(int langId);

	const wxString GetResourceDirectory();
	const wxString GetPackageDirectory();

	unsigned GetEventCount();
	wxString GetEventGroup(unsigned index);
	wxString GetEventTitle(unsigned index);
	GOrgueMidiReceiverBase* GetMidiEvent(unsigned index);
	GOrgueMidiReceiverBase* FindMidiEvent(MIDI_RECEIVER_TYPE type, unsigned index);

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

	const GOrgueSoundPortsConfig & GetPortsConfig() const
	{ return m_PortsConfig; }

	void SetPortsConfig(const GOrgueSoundPortsConfig & portsConfig)
	{ m_PortsConfig = portsConfig; }

	const std::vector<GOAudioDeviceConfig>& GetAudioDeviceConfig();
	const unsigned GetTotalAudioChannels() const;
	void SetAudioDeviceConfig(const std::vector<GOAudioDeviceConfig>& config);
	unsigned GetDefaultLatency();

	GOrgueMidiMap& GetMidiMap();

	GOrgueTemperamentList& GetTemperaments();

	wxRect GetMainWindowRect();
	void SetMainWindowRect(const wxRect &rect);

	void Flush();
};

#endif
