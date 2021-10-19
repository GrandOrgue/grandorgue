/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPIPECONFIG_H
#define GORGUEPIPECONFIG_H

#include "GOrguePipeUpdateCallback.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GrandOrgueFile;

class GOrguePipeConfig
{
private:
	GrandOrgueFile* m_OrganFile;
	// Note: Call items must be listed in GOrguePipeUpdateGallback.h in the "core" folder.
	GOrguePipeUpdateCallback* m_Callback;
	wxString m_Group;
	wxString m_NamePrefix;
	wxString m_AudioGroup;
	float m_Amplitude;
	float m_DefaultAmplitude;
	float m_Gain;
	float m_DefaultGain;
	float m_Tuning;
	float m_DefaultTuning;
	unsigned m_Delay;
	unsigned m_DefaultDelay;
	int m_ReleaseTruncationLength;
	int m_DefaultReleaseTruncationLength;
	int m_BitsPerSample;
	int m_Compress;
	int m_Channels;
	int m_LoopLoad;
	int m_AttackLoad;
	int m_ReleaseLoad;

public:
	GOrguePipeConfig(GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback);

	void Init(GOrgueConfigReader& cfg, wxString group, wxString prefix);
	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
	void Save(GOrgueConfigWriter& cfg);

	GOrguePipeUpdateCallback* GetCallback();

	float GetAmplitude();
	float GetDefaultAmplitude();
	void SetAmplitude(float amp);

	float GetGain();
	float GetDefaultGain();
	void SetGain(float gain);

	float GetTuning();
	float GetDefaultTuning();
	void SetTuning(float cent);

	unsigned GetDelay();
	unsigned GetDefaultDelay();
	void SetDelay(unsigned delay);

	unsigned GetReleaseTruncationLength();
	unsigned GetDefaultReleaseTruncationLength();
	void SetReleaseTruncationLength(unsigned truncation);

	const wxString& GetAudioGroup();
	void SetAudioGroup(const wxString& str);

	int GetBitsPerSample();
	void SetBitsPerSample(int value);

	int GetCompress();
	void SetCompress(int value);

	int GetChannels();
	void SetChannels(int value);

	int GetLoopLoad();
	void SetLoopLoad(int value);

	int GetAttackLoad();
	void SetAttackLoad(int value);

	int GetReleaseLoad();
	void SetReleaseLoad(int value);
};

#endif
