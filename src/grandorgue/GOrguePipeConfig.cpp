/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
    m_ReleaseTruncationLength(0),
    m_DefaultReleaseTruncationLength(0),
	m_BitsPerSample(-1),
	m_Compress(-1),
	m_Channels(-1),
	m_LoopLoad(-1),
	m_AttackLoad(-1),
	m_ReleaseLoad(-1)
{
}

void GOrguePipeConfig::Init(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	m_Group = group;
	m_NamePrefix = prefix;
	m_AudioGroup = cfg.ReadString(CMBSetting, group, prefix + wxT("AudioGroup"), false);
	m_DefaultAmplitude = 100;
	m_Amplitude = cfg.ReadFloat(CMBSetting, group, prefix + wxT("Amplitude"), 0, 1000, false, m_DefaultAmplitude);
	m_DefaultGain = 0;
	m_Gain = cfg.ReadFloat(CMBSetting, group, prefix + wxT("UserGain"), -120, 40, false, m_DefaultGain);
	m_DefaultTuning = 0;
	m_Tuning = cfg.ReadFloat(CMBSetting, group, prefix + wxT("Tuning"), -1800, 1800, false, m_DefaultTuning);
	m_DefaultDelay = 0;
	m_Delay = cfg.ReadInteger(CMBSetting, group, prefix + wxT("Delay"), 0, 10000, false, m_DefaultDelay);
    m_DefaultReleaseTruncationLength = 0;
    m_ReleaseTruncationLength = cfg.ReadInteger(CMBSetting, group, prefix + wxT("ReleaseTruncationLength"), 0, 10000, false, m_DefaultReleaseTruncationLength);
	m_BitsPerSample = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("BitsPerSample"), -1, 24, false, -1);
	if (m_BitsPerSample < 8 || m_BitsPerSample > 24)
		m_BitsPerSample = -1;
	m_Compress = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("Compress"), -1, 1, false, -1);
	m_Channels = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("Channels"), -1, 2, false, -1);
	m_LoopLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("LoopLoad"), -1, 2, false, -1);
	m_AttackLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("AttackLoad"), -1, 1, false, -1);
	m_ReleaseLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("ReleaseLoad"), -1, 1, false, -1);
	// Call items must be listed in GOrguePipeUpdateCallback.h in the "core" folder.
	m_Callback->UpdateAmplitude();
	m_Callback->UpdateTuning();
	m_Callback->UpdateReleaseTruncationLength(); // Update Release Truncation Length Member Value
	m_Callback->UpdateAudioGroup();
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
	m_DefaultTuning = cfg.ReadFloat(ODFSetting, group, prefix + wxT("PitchTuning"), -1800, 1800, false, 0);
	m_Tuning = cfg.ReadFloat(CMBSetting, group, prefix + wxT("Tuning"), -1800, 1800, false, m_DefaultTuning);
	m_DefaultDelay = cfg.ReadInteger(ODFSetting, group, prefix + wxT("TrackerDelay"), 0, 10000, false, 0);
	m_Delay = cfg.ReadInteger(CMBSetting, group, prefix + wxT("Delay"), 0, 10000, false, m_DefaultDelay);
	/* Release Sample Truncation Settings
     * Default Settings Pulled from ODF. Numeric Range = 0-10000. If no value exists in ODF, default value = 0.  */
    m_DefaultReleaseTruncationLength = cfg.ReadInteger(ODFSetting, group, prefix + wxT("ReleaseTruncation"), 0, 10000, false, 0);
    /* Pulls Settings Set by User in GUI Organ Settings Panel. */
    m_ReleaseTruncationLength = cfg.ReadInteger(CMBSetting, group, prefix + wxT("ReleaseTruncationLength"), 0, 10000, false, m_DefaultReleaseTruncationLength);
	m_BitsPerSample = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("BitsPerSample"), -1, 24, false, -1);
	if (m_BitsPerSample < 8 || m_BitsPerSample > 24)
		m_BitsPerSample = -1;
	m_Compress = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("Compress"), -1, 1, false, -1);
	m_Channels = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("Channels"), -1, 2, false, -1);
	m_LoopLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("LoopLoad"), -1, 2, false, -1);
	m_AttackLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("AttackLoad"), -1, 1, false, -1);
	m_ReleaseLoad = cfg.ReadInteger(CMBSetting, m_Group, m_NamePrefix + wxT("ReleaseLoad"), -1, 1, false, -1);
	// Call items must be listed in GOrguePipeUpdateCallback.h in the "core" folder.
	m_Callback->UpdateAmplitude();
	m_Callback->UpdateTuning();
	m_Callback->UpdateReleaseTruncationLength(); // Update Release Truncation Length Member Value
	m_Callback->UpdateAudioGroup();
}

void GOrguePipeConfig::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteString(m_Group, m_NamePrefix + wxT("AudioGroup"), m_AudioGroup);
	cfg.WriteFloat(m_Group, m_NamePrefix + wxT("Amplitude"), m_Amplitude);
	cfg.WriteFloat(m_Group, m_NamePrefix + wxT("UserGain"), m_Gain);
	cfg.WriteFloat(m_Group, m_NamePrefix + wxT("Tuning"), m_Tuning);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Delay"), m_Delay);
    cfg.WriteInteger(m_Group, m_NamePrefix + wxT("ReleaseTruncationLength"), m_ReleaseTruncationLength);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("BitsPerSample"), m_BitsPerSample);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Compress"), m_Compress);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Channels"), m_Channels);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("LoopLoad"), m_LoopLoad);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("AttackLoad"), m_AttackLoad);
	cfg.WriteInteger(m_Group, m_NamePrefix + wxT("ReleaseLoad"), m_ReleaseLoad);
}

GOrguePipeUpdateCallback* GOrguePipeConfig::GetCallback()
{
	return m_Callback;
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
	if (cent < -1800)
		cent = -1800;
	if (cent > 1800)
		cent = 1800;
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

/*  Release Sample Truncation Settings
 *  Links to OrganDialog.cpp */
unsigned GOrguePipeConfig::GetReleaseTruncationLength()
{
    return m_ReleaseTruncationLength;
}

unsigned GOrguePipeConfig::GetDefaultReleaseTruncationLength()
{
    return m_DefaultReleaseTruncationLength;
}

void GOrguePipeConfig::SetReleaseTruncationLength(unsigned truncation)
{
    m_ReleaseTruncationLength = truncation;
    m_OrganFile->Modified();
    // Call items must be listed in GOrguePipeUpdateCallback.h in the "core" folder.
    m_Callback->UpdateReleaseTruncationLength();
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
