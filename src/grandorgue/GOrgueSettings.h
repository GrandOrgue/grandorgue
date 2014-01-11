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

#ifndef GORGUESETTINGS_H
#define GORGUESETTINGS_H

#include "ptrvector.h"
#include "GOrgueOrgan.h"
#include <wx/string.h>
#include <map>
#include <vector>

class wxConfigBase;

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
	std::vector< std::vector<GOAudioGroupOutputConfig> > scale_factors;
} GOAudioDeviceConfig;

typedef struct
{
	MIDI_RECEIVER_TYPE type;
	unsigned index;
	const wxString group;
	const wxString name;
} GOMidiSetting;

class GOrgueSettings {
private:
	wxConfigBase& m_Config;
	wxString m_InstanceName;
	wxString m_ConfigFileName;
	size_t m_MemoryLimit;
	bool m_Stereo;
	unsigned m_Concurrency;
	unsigned m_ReleaseConcurrency;
	unsigned m_LoadConcurrency;
	bool m_LosslessCompression;
	bool m_ManagePolyphony;
	bool m_ManageCache;
	bool m_CompressCache;
	bool m_ScaleRelease;
	bool m_RandomizeSpeaking;
	bool m_LoadLastFile;
	bool m_ODFCheck;
	unsigned m_SamplesPerBuffer;
	unsigned m_SampleRate;
	unsigned m_BitsPerSample;
	unsigned m_InterpolationType;
	unsigned m_WaveFormat;
	unsigned m_LoopLoad;
	unsigned m_AttackLoad;
	unsigned m_ReleaseLoad;
	int m_Volume;
	unsigned m_PolyphonyLimit;
	unsigned m_Preset;
	ptr_vector<GOrgueOrgan> m_OrganList;
	std::map<wxString, bool> m_MidiIn;
	std::map<wxString, unsigned> m_MidiInShift;
	std::map<wxString, bool> m_MidiOut;
	wxString m_WAVPath;
	wxString m_OrganPath;
	wxString m_SettingPath;
	wxString m_UserSettingPath;
	wxString m_UserCachePath;
	wxString m_LastFile;
	std::vector<wxString> m_AudioGroups;
	std::vector<GOAudioDeviceConfig> m_AudioDeviceConfig;
	int m_Transpose;
	int m_ReleaseLength;
	bool m_ReverbEnabled;
	bool m_ReverbDirect;
	int m_ReverbChannel;
	unsigned m_ReverbStartOffset;
	unsigned m_ReverbLen;
	unsigned m_ReverbDelay;
	float m_ReverbGain;
	wxString m_ReverbFile;
	ptr_vector<GOrgueMidiReceiver> m_MIDIEvents;

	static const GOMidiSetting m_MIDISettings[];

	wxString GetEventSection(unsigned index);

public:
	GOrgueSettings(wxString instance);
	~GOrgueSettings();

	void Load();
	wxConfigBase& GetConfig();
	wxString GetStandardDocumentDirectory();
	wxString GetStandardOrganDirectory();
	wxString GetStandardDataDirectory();
	wxString GetStandardCacheDirectory();

	size_t GetMemoryLimit();
	void SetMemoryLimit(size_t limit);

	unsigned GetEventCount();
	wxString GetEventGroup(unsigned index);
	wxString GetEventTitle(unsigned index);
	GOrgueMidiReceiver* GetMidiEvent(unsigned index);
	GOrgueMidiReceiver* FindMidiEvent(MIDI_RECEIVER_TYPE type, unsigned index);
	
	wxString GetOrganPath();
	void SetOrganPath(wxString path);
	wxString GetSettingPath();
	void SetSettingPath(wxString path);
	wxString GetWAVPath();
	void SetWAVPath(wxString path);

	wxString GetUserSettingPath();
	void SetUserSettingPath(wxString path);
	wxString GetUserCachePath();
	void SetUserCachePath(wxString path);
	wxString GetLastFile();
	void SetLastFile(wxString path);
	unsigned GetPreset();
	void SetPreset(unsigned value);
	bool GetODFCheck();
	void SetODFCheck(bool strict);

	bool GetLoadInStereo();
	void SetLoadInStereo(bool stereo);

	unsigned GetConcurrency();
	void SetConcurrency(unsigned concurrency);
	unsigned GetReleaseConcurrency();
	void SetReleaseConcurrency(unsigned concurrency);
	unsigned GetLoadConcurrency();
	void SetLoadConcurrency(unsigned concurrency);

	unsigned GetSamplesPerBuffer();
	void SetSamplesPerBuffer(unsigned sampler_per_buffer);

	bool GetLosslessCompression();
	void SetLosslessCompression(bool lossless_compression);
	bool GetManagePolyphony();
	void SetManagePolyphony(bool manage_polyphony);
	bool GetManageCache();
	void SetManageCache(bool manage);
	bool GetCompressCache();
	void SetCompressCache(bool compress);
	bool GetScaleRelease();
	void SetScaleRelease(bool scale_release);
	bool GetRandomizeSpeaking();
	void SetRandomizeSpeaking(bool randomize);
	bool GetLoadLastFile();
	void SetLoadLastFile(bool last_file);
	unsigned GetInterpolationType();
	void SetInterpolationType(unsigned type);
	unsigned GetSampleRate();
	void SetSampleRate(unsigned sample_rate);
	unsigned GetWaveFormatBytesPerSample();
	void SetWaveFormatBytesPerSample(unsigned bytes_per_sample);
	unsigned GetBitsPerSample();
	void SetBitsPerSample(unsigned bits_per_sample);

	unsigned GetLoopLoad();
	void SetLoopLoad(unsigned loop_load);
	unsigned GetAttackLoad();
	void SetAttackLoad(unsigned attack_load);
	unsigned GetReleaseLoad();
	void SetReleaseLoad(unsigned release_load);

	int GetVolume();
	void SetVolume(int volume);
	unsigned GetPolyphonyLimit();
	void SetPolyphonyLimit(unsigned polyphony_limit);

	unsigned GetAudioDeviceLatency(wxString device);
	void SetAudioDeviceLatency(wxString device, unsigned latency);
	int GetAudioDeviceActualLatency(wxString device);
	void SetAudioDeviceActualLatency(wxString device, unsigned latency);

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

	int GetTranspose();
	void SetTranspose(int transpose);
	unsigned GetReleaseLength();
	void SetReleaseLength(unsigned reverb);

	bool GetReverbEnabled();
	void SetReverbEnabled(bool on);
	bool GetReverbDirect();
	void SetReverbDirect(bool on);
	wxString GetReverbFile();
	void SetReverbFile(wxString file);
	unsigned GetReverbStartOffset();
	void SetReverbStartOffset(unsigned offset);
	unsigned GetReverbLen();
	void SetReverbLen(unsigned length);
	float GetReverbGain();
	void SetReverbGain(float gain);
	int GetReverbChannel();
	void SetReverbChannel(int channel);
	unsigned GetReverbDelay();
	void SetReverbDelay(unsigned delay);

	void AddOrgan(GOrgueOrgan* organ);
	ptr_vector<GOrgueOrgan>& GetOrganList();
	std::vector<GOrgueOrgan*> GetLRUOrganList();

	void Flush();
};

#endif
