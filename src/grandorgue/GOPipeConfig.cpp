/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipeConfig.h"

#include "GODefinitionFile.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOPipeConfig::GOPipeConfig(
  GODefinitionFile *organfile, GOPipeUpdateCallback *callback)
    : m_OrganFile(organfile),
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
      m_ReleaseLoad(-1) {}

void GOPipeConfig::Init(GOConfigReader &cfg, wxString group, wxString prefix) {
  m_Group = group;
  m_NamePrefix = prefix;
  m_AudioGroup
    = cfg.ReadString(CMBSetting, group, prefix + wxT("AudioGroup"), false);
  m_DefaultAmplitude = 100;
  m_Amplitude = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("Amplitude"),
    0,
    1000,
    false,
    m_DefaultAmplitude);
  m_DefaultGain = 0;
  m_Gain = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("UserGain"),
    -120,
    40,
    false,
    m_DefaultGain);
  m_DefaultTuning = 0;
  m_Tuning = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("Tuning"),
    -1800,
    1800,
    false,
    m_DefaultTuning);
  m_DefaultDelay = 0;
  m_Delay = cfg.ReadInteger(
    CMBSetting, group, prefix + wxT("Delay"), 0, 10000, false, m_DefaultDelay);
  m_BitsPerSample = cfg.ReadInteger(
    CMBSetting,
    m_Group,
    m_NamePrefix + wxT("BitsPerSample"),
    -1,
    24,
    false,
    -1);
  if (m_BitsPerSample < 8 || m_BitsPerSample > 24) m_BitsPerSample = -1;
  m_Compress = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("Compress"), -1, 1, false, -1);
  m_Channels = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("Channels"), -1, 2, false, -1);
  m_LoopLoad = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("LoopLoad"), -1, 2, false, -1);
  m_AttackLoad = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("AttackLoad"), -1, 1, false, -1);
  m_ReleaseLoad = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("ReleaseLoad"), -1, 1, false, -1);
  m_Callback->UpdateAmplitude();
  m_Callback->UpdateTuning();
  m_Callback->UpdateAudioGroup();
}

void GOPipeConfig::Load(GOConfigReader &cfg, wxString group, wxString prefix) {
  m_Group = group;
  m_NamePrefix = prefix;
  m_AudioGroup
    = cfg.ReadString(CMBSetting, group, prefix + wxT("AudioGroup"), false);
  m_DefaultAmplitude = cfg.ReadFloat(
    ODFSetting, group, prefix + wxT("AmplitudeLevel"), 0, 1000, false, 100);
  m_Amplitude = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("Amplitude"),
    0,
    1000,
    false,
    m_DefaultAmplitude);
  m_DefaultGain = cfg.ReadFloat(
    ODFSetting, group, prefix + wxT("Gain"), -120, 40, false, 0);
  m_Gain = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("UserGain"),
    -120,
    40,
    false,
    m_DefaultGain);
  m_DefaultTuning = cfg.ReadFloat(
    ODFSetting, group, prefix + wxT("PitchTuning"), -1800, 1800, false, 0);
  m_Tuning = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("Tuning"),
    -1800,
    1800,
    false,
    m_DefaultTuning);
  m_DefaultDelay = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("TrackerDelay"), 0, 10000, false, 0);
  m_Delay = cfg.ReadInteger(
    CMBSetting, group, prefix + wxT("Delay"), 0, 10000, false, m_DefaultDelay);
  m_BitsPerSample = cfg.ReadInteger(
    CMBSetting,
    m_Group,
    m_NamePrefix + wxT("BitsPerSample"),
    -1,
    24,
    false,
    -1);
  if (m_BitsPerSample < 8 || m_BitsPerSample > 24) m_BitsPerSample = -1;
  m_Compress = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("Compress"), -1, 1, false, -1);
  m_Channels = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("Channels"), -1, 2, false, -1);
  m_LoopLoad = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("LoopLoad"), -1, 2, false, -1);
  m_AttackLoad = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("AttackLoad"), -1, 1, false, -1);
  m_ReleaseLoad = cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("ReleaseLoad"), -1, 1, false, -1);
  m_Callback->UpdateAmplitude();
  m_Callback->UpdateTuning();
  m_Callback->UpdateAudioGroup();
}

void GOPipeConfig::Save(GOConfigWriter &cfg) {
  cfg.WriteString(m_Group, m_NamePrefix + wxT("AudioGroup"), m_AudioGroup);
  cfg.WriteFloat(m_Group, m_NamePrefix + wxT("Amplitude"), m_Amplitude);
  cfg.WriteFloat(m_Group, m_NamePrefix + wxT("UserGain"), m_Gain);
  cfg.WriteFloat(m_Group, m_NamePrefix + wxT("Tuning"), m_Tuning);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Delay"), m_Delay);
  cfg.WriteInteger(
    m_Group, m_NamePrefix + wxT("BitsPerSample"), m_BitsPerSample);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Compress"), m_Compress);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Channels"), m_Channels);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("LoopLoad"), m_LoopLoad);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("AttackLoad"), m_AttackLoad);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("ReleaseLoad"), m_ReleaseLoad);
}

GOPipeUpdateCallback *GOPipeConfig::GetCallback() { return m_Callback; }

const wxString &GOPipeConfig::GetAudioGroup() { return m_AudioGroup; }

void GOPipeConfig::SetAudioGroup(const wxString &str) {
  m_AudioGroup = str;
  m_OrganFile->Modified();
  m_Callback->UpdateAudioGroup();
}

float GOPipeConfig::GetAmplitude() { return m_Amplitude; }

float GOPipeConfig::GetDefaultAmplitude() { return m_DefaultAmplitude; }

void GOPipeConfig::SetAmplitude(float amp) {
  m_Amplitude = amp;
  m_OrganFile->Modified();
  m_Callback->UpdateAmplitude();
}

float GOPipeConfig::GetGain() { return m_Gain; }

float GOPipeConfig::GetDefaultGain() { return m_DefaultGain; }

void GOPipeConfig::SetGain(float gain) {
  m_Gain = gain;
  m_OrganFile->Modified();
  m_Callback->UpdateAmplitude();
}

float GOPipeConfig::GetTuning() { return m_Tuning; }

float GOPipeConfig::GetDefaultTuning() { return m_DefaultTuning; }

void GOPipeConfig::SetTuning(float cent) {
  if (cent < -1800) cent = -1800;
  if (cent > 1800) cent = 1800;
  m_Tuning = cent;
  m_OrganFile->Modified();
  m_Callback->UpdateTuning();
}

unsigned GOPipeConfig::GetDelay() { return m_Delay; }

unsigned GOPipeConfig::GetDefaultDelay() { return m_DefaultDelay; }

void GOPipeConfig::SetDelay(unsigned delay) {
  m_Delay = delay;
  m_OrganFile->Modified();
}

int GOPipeConfig::GetBitsPerSample() { return m_BitsPerSample; }

void GOPipeConfig::SetBitsPerSample(int value) {
  m_BitsPerSample = value;
  m_OrganFile->Modified();
}

int GOPipeConfig::GetCompress() { return m_Compress; }

void GOPipeConfig::SetCompress(int value) {
  m_Compress = value;
  m_OrganFile->Modified();
}

int GOPipeConfig::GetChannels() { return m_Channels; }

void GOPipeConfig::SetChannels(int value) {
  m_Channels = value;
  m_OrganFile->Modified();
}

int GOPipeConfig::GetLoopLoad() { return m_LoopLoad; }

void GOPipeConfig::SetLoopLoad(int value) {
  m_LoopLoad = value;
  m_OrganFile->Modified();
}

int GOPipeConfig::GetAttackLoad() { return m_AttackLoad; }

void GOPipeConfig::SetAttackLoad(int value) {
  m_AttackLoad = value;
  m_OrganFile->Modified();
}

int GOPipeConfig::GetReleaseLoad() { return m_ReleaseLoad; }

void GOPipeConfig::SetReleaseLoad(int value) {
  m_ReleaseLoad = value;
  m_OrganFile->Modified();
}
