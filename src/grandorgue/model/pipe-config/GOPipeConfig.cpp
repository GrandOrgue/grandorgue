/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipeConfig.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOPipeConfig::GOPipeConfig(
  GOPipeConfigListener &listener, GOPipeUpdateCallback *callback)
  : r_listener(listener),
    m_Callback(callback),
    m_Group(),
    m_NamePrefix(),
    m_AudioGroup(),
    m_DefaultAmplitude(0),
    m_Amplitude(0),
    m_DefaultGain(0),
    m_Gain(0),
    m_PitchTuning(0),
    m_PitchCorrection(0),
    m_ManualTuning(0),
    m_AutoTuningCorrection(0),
    m_DefaultDelay(0),
    m_Delay(0),
    m_ReleaseTail(0),
    m_BitsPerSample(-1),
    m_Channels(-1),
    m_LoopLoad(-1),
    m_Percussive(BOOL3_DEFAULT),
    m_IndependentRelease(BOOL3_DEFAULT),
    m_Compress(BOOL3_DEFAULT),
    m_AttackLoad(BOOL3_DEFAULT),
    m_ReleaseLoad(BOOL3_DEFAULT),
    m_IgnorePitch(BOOL3_DEFAULT) {}

static const wxString WX_TUNING = wxT("Tuning");
static const wxString WX_MANUAL_TUNING = wxT("ManualTuning");
static const wxString WX_AUTO_TUNING_CORRECTION = wxT("AutoTuningCorrection");

void GOPipeConfig::LoadFromCmb(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  m_Group = group;
  m_NamePrefix = prefix;
  m_AudioGroup
    = cfg.ReadString(CMBSetting, group, prefix + wxT("AudioGroup"), false);
  m_Amplitude = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("Amplitude"),
    0,
    1000,
    false,
    m_DefaultAmplitude);
  m_Gain = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + wxT("UserGain"),
    -120,
    40,
    false,
    m_DefaultGain);
  // m_PitchTuning must be read before calling GOPipeConfig::ReadTuning
  float legacyTuning = cfg.ReadFloat(
    CMBSetting, group, prefix + WX_TUNING, -1800, 1800, false, m_PitchTuning);

  // m_PitchTuning must be read before calling GOPipeConfig::ReadTuning
  m_ManualTuning = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + WX_MANUAL_TUNING,
    -1800,
    1800,
    false,
    legacyTuning - m_PitchTuning);
  m_AutoTuningCorrection = cfg.ReadFloat(
    CMBSetting,
    group,
    prefix + WX_AUTO_TUNING_CORRECTION,
    -1800,
    1800,
    false,
    0);
  m_Delay = (uint16_t)cfg.ReadInteger(
    CMBSetting, group, prefix + wxT("Delay"), 0, 10000, false, m_DefaultDelay);
  m_BitsPerSample = (int8_t)cfg.ReadInteger(
    CMBSetting,
    m_Group,
    m_NamePrefix + wxT("BitsPerSample"),
    -1,
    24,
    false,
    -1);
  if (m_BitsPerSample < 8 || m_BitsPerSample > 24)
    m_BitsPerSample = -1;
  m_Channels = (int8_t)cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("Channels"), -1, 2, false, -1);
  m_LoopLoad = (int8_t)cfg.ReadInteger(
    CMBSetting, m_Group, m_NamePrefix + wxT("LoopLoad"), -1, 2, false, -1);
  m_Compress = cfg.ReadBool3FromInt(
    CMBSetting, m_Group, m_NamePrefix + wxT("Compress"), false);
  m_AttackLoad = cfg.ReadBool3FromInt(
    CMBSetting, m_Group, m_NamePrefix + wxT("AttackLoad"), false);
  m_ReleaseLoad = cfg.ReadBool3FromInt(
    CMBSetting, m_Group, m_NamePrefix + wxT("ReleaseLoad"), false);
  m_IgnorePitch = cfg.ReadBooleanTriple(
    CMBSetting, m_Group, m_NamePrefix + wxT("IgnorePitch"), false);
  m_ReleaseTail = (uint16_t)cfg.ReadInteger(
    CMBSetting, group, m_NamePrefix + wxT("ReleaseTail"), 0, 3000, false, 0);

  m_Callback->UpdateAmplitude();
  m_Callback->UpdateTuning();
  m_Callback->UpdateAudioGroup();
  m_Callback->UpdateReleaseTail();
}

void GOPipeConfig::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  m_DefaultAmplitude = 100;
  m_DefaultGain = 0;
  m_PitchTuning = 0;
  m_PitchCorrection = 0;
  m_DefaultDelay = 0;
  m_Percussive = BOOL3_DEFAULT;
  m_IndependentRelease = BOOL3_DEFAULT;
  LoadFromCmb(cfg, group, prefix);
}

void GOPipeConfig::Load(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &prefix,
  bool isParentPercussive) {
  m_DefaultAmplitude = cfg.ReadFloat(
    ODFSetting, group, prefix + wxT("AmplitudeLevel"), 0, 1000, false, 100);
  m_DefaultGain = cfg.ReadFloat(
    ODFSetting, group, prefix + wxT("Gain"), -120, 40, false, 0);
  m_PitchTuning = cfg.ReadFloat(
    ODFSetting, group, prefix + wxT("PitchTuning"), -1800, 1800, false, 0);
  m_PitchCorrection = cfg.ReadFloat(
    ODFSetting, group, prefix + wxT("PitchCorrection"), -1800, 1800, false, 0);
  m_DefaultDelay = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("TrackerDelay"), 0, 10000, false, 0);
  m_Percussive = cfg.ReadBooleanTriple(
    ODFSetting, group, prefix + wxT("Percussive"), false);
  m_IndependentRelease = to_bool(m_Percussive, isParentPercussive)
    ? cfg.ReadBooleanTriple(
      ODFSetting, group, prefix + wxT("HasIndependentRelease"), false)
    : BOOL3_DEFAULT;
  LoadFromCmb(cfg, group, prefix);
}

void GOPipeConfig::Save(GOConfigWriter &cfg) {
  cfg.WriteString(m_Group, m_NamePrefix + wxT("AudioGroup"), m_AudioGroup);
  cfg.WriteFloat(m_Group, m_NamePrefix + wxT("Amplitude"), m_Amplitude);
  cfg.WriteFloat(m_Group, m_NamePrefix + wxT("UserGain"), m_Gain);
  cfg.WriteFloat(m_Group, m_NamePrefix + WX_MANUAL_TUNING, m_ManualTuning);
  cfg.WriteFloat(
    m_Group, m_NamePrefix + WX_AUTO_TUNING_CORRECTION, m_AutoTuningCorrection);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Delay"), m_Delay);
  cfg.WriteInteger(
    m_Group, m_NamePrefix + wxT("BitsPerSample"), m_BitsPerSample);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Compress"), m_Compress);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("Channels"), m_Channels);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("LoopLoad"), m_LoopLoad);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("AttackLoad"), m_AttackLoad);
  cfg.WriteInteger(m_Group, m_NamePrefix + wxT("ReleaseLoad"), m_ReleaseLoad);
  cfg.WriteBooleanTriple(
    m_Group, m_NamePrefix + wxT("IgnorePitch"), m_IgnorePitch);
  cfg.WriteInteger(
    m_Group, m_NamePrefix + wxT("ReleaseTail"), (int)m_ReleaseTail);
}

void GOPipeConfig::SetPitchMember(float cents, float &member) {
  if (cents < -1800)
    cents = -1800;
  if (cents > 1800)
    cents = 1800;
  SetSmallMember(cents, member, &GOPipeUpdateCallback::UpdateTuning);
}
