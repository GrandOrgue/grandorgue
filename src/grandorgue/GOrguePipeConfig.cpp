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

#include "GOrguePipeConfig.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GrandOrgueFile.h"

GOrguePipeConfig::GOrguePipeConfig(GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback) :
	m_OrganFile(organfile),
	m_Callback(callback),
	m_Group(),
	m_NamePrefix(),
	m_AudioGroup(),
	m_Amplitude(0),
	m_DefaultAmplitude(0),
	m_Gain(0),
	m_DefaultGain(0),
	m_Tuning(0),
	m_DefaultTuning(0),
	m_Delay(0),
	m_DefaultDelay(0),
	m_BitsPerSample(-1),
	m_Compress(-1),
	m_Channels(-1),
	m_LoopLoad(-1),
	m_AttackLoad(-1),
	m_ReleaseLoad(-1)
{
}

void GOrguePipeConfig::Load(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	m_Group = group;
	m_NamePrefix = prefix;
	m_AudioGroup = cfg.ReadString(CMBSetting, group, prefix + wxT("AudioGroup"), false);
	m_DefaultAmplitude = cfg.ReadFloat(ODFSetting, group, prefix + wxT("AmplitudeLevel"), 0, 1000, false, 100);
	m_Amplitude = cfg.ReadFloat(CMBSetting, group, prefix + wxT("Amplitude"), 0, 1000, false, m_DefaultAmplitude);
	m_DefaultGain = cfg.ReadFloat(ODFSetting, group, prefix + wxT("Gain"), -120, 40, false, 0);
	m_Gain = cfg.ReadFloat(CMBSetting, group, prefix + wxT("UserGain"), -120, 40, false, m_DefaultGain);
	m_DefaultTuning = cfg.ReadFloat(ODFSetting, group, prefix + wxT("PitchTuning"), -1200, 1200, false, 0);
	m_Tuning = cfg.ReadFloat(CMBSetting, group, prefix + wxT("Tuning"), -1200, 1200, false, m_DefaultTuning);
	m_DefaultDelay = cfg.ReadInteger(ODFSetting, group, prefix + wxT("TrackerDelay"), 0, 10000, false, 0);
	m_Delay = cfg.ReadInteger(CMBSetting, group, prefix + wxT("Delay"), 0, 10000, false, m_DefaultDelay);
	m_BitsPerSample = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("BitsPerSample"), -1, 24, false, -1);
	if (m_BitsPerSample != 8 && m_BitsPerSample != 12 && m_BitsPerSample != 16 && m_BitsPerSample != 20 && m_BitsPerSample != 24)
		m_BitsPerSample = -1;
	m_Compress = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("Compress"), -1, 1, false, -1);
	m_Channels = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("Channels"), -1, 2, false, -1);
	m_LoopLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("LoopLoad"), -1, 2, false, -1);
	m_AttackLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("AttackLoad"), -1, 1, false, -1);
	m_ReleaseLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("ReleaseLoad"), -1, 1, false, -1);
	m_Callback->UpdateAmplitude();
	m_Callback->UpdateTuning();
	m_Callback->UpdateAudioGroup();
}

void GOrguePipeConfig::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteString(m_Group, m_NamePrefix + wxT("AudioGroup"), m_AudioGroup);
	cfg.WriteFloat(m_Group, m_NamePrefix + wxT("Amplitude"), m_Amplitude);
	cfg.WriteFloat(m_Group, m_NamePrefix + wxT("UserGain"), m_Gain);
	cfg.WriteFloat(m_Group, m_NamePrefix + wxT("Tuning"), m_Tuning);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Delay"), m_Delay);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("BitsPerSample"), m_BitsPerSample);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Compress"), m_Compress);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Channels"), m_Channels);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("LoopLoad"), m_LoopLoad);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("AttackLoad"), m_AttackLoad);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("ReleaseLoad"), m_ReleaseLoad);
}

const wxString& GOrguePipeConfig::GetAudioGroup()
{
	return m_AudioGroup;
}

void GOrguePipeConfig::SetAudioGroup(const wxString& str)
{
	m_AudioGroup = str;
	m_OrganFile->Modified();
	m_Callback->UpdateAudioGroup();
}

float GOrguePipeConfig::GetAmplitude()
{
	return m_Amplitude;
}

float GOrguePipeConfig::GetDefaultAmplitude()
{
	return m_DefaultAmplitude;
}

void GOrguePipeConfig::SetAmplitude(float amp)
{
	m_Amplitude = amp;
	m_OrganFile->Modified();
	m_Callback->UpdateAmplitude();
}

float GOrguePipeConfig::GetGain()
{
	return m_Gain;
}

float GOrguePipeConfig::GetDefaultGain()
{
	return m_DefaultGain;
}

void GOrguePipeConfig::SetGain(float gain)
{
	m_Gain = gain;
	m_OrganFile->Modified();
	m_Callback->UpdateAmplitude();
}

float GOrguePipeConfig::GetTuning()
{
	return m_Tuning;
}

float GOrguePipeConfig::GetDefaultTuning()
{
	return m_DefaultTuning;
}

void GOrguePipeConfig::SetTuning(float cent)
{
	if (cent < -1200)
		cent = -1200;
	if (cent > 1200)
		cent = 1200;
	m_Tuning = cent;
	m_OrganFile->Modified();
	m_Callback->UpdateTuning();
}

unsigned GOrguePipeConfig::GetDelay()
{
	return m_Delay;
}

unsigned GOrguePipeConfig::GetDefaultDelay()
{
	return m_DefaultDelay;
}

void GOrguePipeConfig::SetDelay(unsigned delay)
{
	m_Delay = delay;
	m_OrganFile->Modified();
}

int  GOrguePipeConfig::GetBitsPerSample()
{
	return m_BitsPerSample;
}

void  GOrguePipeConfig::SetBitsPerSample(int value)
{
	m_BitsPerSample = value;
	m_OrganFile->Modified();
}

int  GOrguePipeConfig::GetCompress()
{
	return m_Compress;
}

void  GOrguePipeConfig::SetCompress(int value)
{
	m_Compress = value;
	m_OrganFile->Modified();
}

int  GOrguePipeConfig::GetChannels()
{
	return m_Channels;
}

void  GOrguePipeConfig::SetChannels(int value)
{
	m_Channels = value;
	m_OrganFile->Modified();
}

int  GOrguePipeConfig::GetLoopLoad()
{
	return m_LoopLoad;
}

void  GOrguePipeConfig::SetLoopLoad(int value)
{
	m_LoopLoad = value;
	m_OrganFile->Modified();
}

int  GOrguePipeConfig::GetAttackLoad()
{
	return m_AttackLoad;
}

void  GOrguePipeConfig::SetAttackLoad(int value)
{
	m_AttackLoad = value;
	m_OrganFile->Modified();
}

int  GOrguePipeConfig::GetReleaseLoad()
{
	return m_ReleaseLoad;
}

void  GOrguePipeConfig::SetReleaseLoad(int value)
{
	m_ReleaseLoad = value;
	m_OrganFile->Modified();
}
