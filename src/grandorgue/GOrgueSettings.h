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

#ifndef GORGUESETTINGS_H
#define GORGUESETTINGS_H

#include <wx/wx.h>
#include <map>
#include <vector>
class wxConfigBase;

class GOrgueSettings {
private:
	wxConfigBase& m_Config;
	bool m_Stereo;
	unsigned m_Concurrency;
	unsigned m_ReleaseConcurrency;
	bool m_LosslessCompression;
	bool m_ManagePolyphony;
	bool m_CompressCache;
	bool m_ScaleRelease;
	bool m_RandomizeSpeaking;
	unsigned m_SampleRate;
	unsigned m_BytesPerSample;
	unsigned m_WaveFormat;
	unsigned m_Volume;
	unsigned m_PolyphonyLimit;
	wxString m_DefaultAudioDevice;
	std::map<long, wxString> m_OrganMidiEvents;
	wxString m_WAVPath;
	wxString m_OrganPath;
	wxString m_SettingPath;
	int m_ManualEvents[6];
	int m_EnclosureEvents[6];
	int m_SetterEvents[3];
	int m_StopChangeEvent;
	int m_Transpose;

	static const wxString m_ManualNames[];
	static const wxString m_EnclosureNames[];
	static const wxString m_SetterNames[];
	static const wxString m_StopChangeName;

public:
	GOrgueSettings();

	void Load();
	wxConfigBase& GetConfig();
	wxString GetStandardDocumentDirectory();
	wxString GetStandardOrganDirectory();

	unsigned GetManualCount();
	wxString GetManualTitle(unsigned index);
	int GetManualEvent(unsigned index);
	void SetManualEvent(unsigned index, int event);

	unsigned GetEnclosureCount();
	wxString GetEnclosureTitle(unsigned index);
	int GetEnclosureEvent(unsigned index);
	void SetEnclosureEvent(unsigned index, int event);

	unsigned GetSetterCount();
	wxString GetSetterTitle(unsigned index);
	int GetSetterEvent(unsigned index);
	void SetSetterEvent(unsigned index, int event);
	
	wxString GetStopChangeTitle();
	int GetStopChangeEvent();
	void SetStopChangeEvent(int event);

	wxString GetOrganPath();
	void SetOrganPath(wxString path);
	wxString GetSettingPath();
	void SetSettingPath(wxString path);
	wxString GetWAVPath();
	void SetWAVPath(wxString path);

	bool GetLoadInStereo();
	void SetLoadInStereo(bool stereo);

	unsigned GetConcurrency();
	void SetConcurrency(unsigned concurrency);
	unsigned GetReleaseConcurrency();
	void SetReleaseConcurrency(unsigned concurrency);

	bool GetLosslessCompression();
	void SetLosslessCompression(bool lossless_compression);
	bool GetManagePolyphony();
	void SetManagePolyphony(bool manage_polyphony);
	bool GetCompressCache();
	void SetCompressCache(bool compress);
	bool GetScaleRelease();
	void SetScaleRelease(bool scale_release);
	bool GetRandomizeSpeaking();
	void SetRandomizeSpeaking(bool randomize);
	unsigned GetSampleRate();
	void SetSampleRate(unsigned sample_rate);
	unsigned GetWaveFormatBytesPerSample();
	void SetWaveFormatBytesPerSample(unsigned bytes_per_sample);
	unsigned GetBytesPerSample();
	void SetBytesPerSample(unsigned bytes_per_sample);

	unsigned GetVolume();
	void SetVolume(unsigned volume);
	unsigned GetPolyphonyLimit();
	void SetPolyphonyLimit(unsigned polyphony_limit);

	const wxString& GetDefaultAudioDevice();
	void SetDefaultAudioDevice(wxString device);
	unsigned GetAudioDeviceLatency(wxString device);
	void SetAudioDeviceLatency(wxString device, unsigned latency);
	int GetAudioDeviceActualLatency(wxString device);
	void SetAudioDeviceActualLatency(wxString device, unsigned latency);

	const std::map<long, wxString>& GetOrganList();
	void SetOrganList(std::map<long, wxString> list);

	int GetMidiDeviceChannelShift(wxString device);
	void SetMidiDeviceChannelShift(wxString device, int shift);
	std::vector<wxString> GetMidiDeviceList();

	int GetTranspose();
	void SetTranspose(int transpose);
};

#endif
