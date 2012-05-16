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

#ifndef GORGUEPIPECONFIG_H
#define GORGUEPIPECONFIG_H

#include <wx/wx.h>
class GOrgueConfigWriter;
class GrandOrgueFile;
class IniFileConfig;

class GOrguePipeUpdateCallback
{
public:
	virtual ~GOrguePipeUpdateCallback() { }
	virtual void UpdateAmplitude() = 0;
	virtual void UpdateTuning() = 0;
	virtual void UpdateAudioGroup() = 0;
};

class GOrguePipeConfig
{
private:
	GrandOrgueFile* m_OrganFile;
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
	int m_BitsPerSample;
	int m_Compress;
	int m_Channels;
	int m_LoopLoad;
	int m_AttackLoad;
	int m_ReleaseLoad;

public:
	GOrguePipeConfig(GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback);

	void Load(IniFileConfig& cfg, wxString group, wxString prefix);
	void Save(GOrgueConfigWriter& cfg);

	float GetAmplitude();
	float GetDefaultAmplitude();
	void SetAmplitude(float amp);

	float GetGain();
	float GetDefaultGain();
	void SetGain(float gain);

	float GetTuning();
	float GetDefaultTuning();
	void SetTuning(float cent);

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
