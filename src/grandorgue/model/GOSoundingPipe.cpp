/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundingPipe.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "temperaments/GOTemperament.h"

#include "GOAlloc.h"
#include "GOHash.h"
#include "GOOrganController.h"
#include "GOPath.h"
#include "GORank.h"
#include "GOWindchest.h"

#include "go_limits.h"

GOSoundingPipe::GOSoundingPipe(
  GOOrganController *organController,
  GORank *rank,
  bool percussive,
  int sampler_group_id,
  unsigned midi_key_number,
  unsigned harmonic_number,
  float min_volume,
  float max_volume,
  bool retune)
  : GOPipe(organController, rank, midi_key_number),
    m_OrganController(organController),
    m_Sampler(NULL),
    m_LastStop(0),
    m_Instances(0),
    m_Tremulant(false),
    m_AttackInfo(),
    m_ReleaseInfo(),
    m_Filename(),
    m_SamplerGroupID(sampler_group_id),
    m_AudioGroupID(0),
    m_Percussive(percussive),
    m_TemperamentOffset(0),
    m_HarmonicNumber(harmonic_number),
    m_LoopCrossfadeLength(0),
    m_ReleaseCrossfadeLength(0),
    m_MinVolume(min_volume),
    m_MaxVolume(max_volume),
    m_OdfMidiKeyNumber(-1),
    m_OdfMidiPitchFraction(-1.0),
    m_SampleMidiKeyNumber(0),
    m_SampleMidiPitchFraction(0.0),
    m_RetunePipe(retune),
    m_IsTemperamentOriginalBased(true),
    m_PipeConfigNode(
      &rank->GetPipeConfig(), organController, this, &m_SoundProvider) {}

void GOSoundingPipe::LoadAttack(
  GOConfigReader &cfg, wxString group, wxString prefix) {
  attack_load_info ainfo;
  ainfo.filename.Assign(cfg.ReadFileName(ODFSetting, group, prefix));
  ainfo.sample_group = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("IsTremulant"), -1, 1, false, -1);
  ainfo.load_release = cfg.ReadBoolean(
    ODFSetting, group, prefix + wxT("LoadRelease"), false, !m_Percussive);
  ;
  ainfo.percussive = m_Percussive;
  ainfo.max_playback_time = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("MaxKeyPressTime"), -1, 100000, false, -1);
  ainfo.cue_point = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("CuePoint"),
    -1,
    MAX_SAMPLE_LENGTH,
    false,
    -1);
  ainfo.min_attack_velocity = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("AttackVelocity"), 0, 127, false, 0);
  ainfo.max_released_time = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("MaxTimeSinceLastRelease"),
    -1,
    100000,
    false,
    -1);
  ainfo.attack_start = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("AttackStart"),
    0,
    MAX_SAMPLE_LENGTH,
    false,
    0);
  ainfo.release_end = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("ReleaseEnd"),
    -1,
    MAX_SAMPLE_LENGTH,
    false,
    -1);

  unsigned loop_cnt = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("LoopCount"), 0, 100, false, 0);
  for (unsigned j = 0; j < loop_cnt; j++) {
    GOWaveLoop linfo;

    linfo.m_StartPosition = cfg.ReadInteger(
      ODFSetting,
      group,
      prefix + wxString::Format(wxT("Loop%03dStart"), j + 1),
      0,
      MAX_SAMPLE_LENGTH,
      false,
      0);
    linfo.m_EndPosition = cfg.ReadInteger(
      ODFSetting,
      group,
      prefix + wxString::Format(wxT("Loop%03dEnd"), j + 1),
      linfo.m_StartPosition + 1,
      MAX_SAMPLE_LENGTH,
      true);
    ainfo.loops.push_back(linfo);
  }

  m_AttackInfo.push_back(ainfo);
}

void GOSoundingPipe::Init(
  GOConfigReader &cfg, wxString group, wxString prefix, wxString filename) {
  m_OrganController->RegisterCacheObject(this);
  m_Filename = filename;
  m_PipeConfigNode.Init(cfg, group, prefix);
  m_OdfMidiKeyNumber = -1;
  m_OdfMidiPitchFraction = -1.0;
  m_LoopCrossfadeLength = 0;
  m_ReleaseCrossfadeLength = 0;
  UpdateAmplitude();
  m_OrganController->GetWindchest(m_SamplerGroupID - 1)->AddPipe(this);

  attack_load_info ainfo;
  ainfo.filename.AssignResource(m_Filename);
  ainfo.sample_group = -1;
  ainfo.load_release = !m_Percussive;
  ainfo.percussive = m_Percussive;
  ainfo.max_playback_time = -1;
  ainfo.cue_point = -1;
  ainfo.min_attack_velocity = 0;
  ainfo.max_released_time = -1;
  ainfo.attack_start = 0;
  ainfo.release_end = -1;
  m_AttackInfo.push_back(ainfo);

  m_SoundProvider.SetVelocityParameter(m_MinVolume, m_MaxVolume);
  m_PipeConfigNode.SetName(
    wxString::Format(_("%d: %s"), m_MidiKeyNumber, m_Filename.c_str()));
}

void GOSoundingPipe::Load(
  GOConfigReader &cfg, wxString group, wxString prefix) {
  m_OrganController->RegisterCacheObject(this);
  m_Filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
  m_PipeConfigNode.Load(cfg, group, prefix);
  m_HarmonicNumber = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("HarmonicNumber"),
    1,
    1024,
    false,
    m_HarmonicNumber);
  m_SamplerGroupID = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("WindchestGroup"),
    1,
    m_OrganController->GetWindchestGroupCount(),
    false,
    m_SamplerGroupID);
  m_Percussive = cfg.ReadBoolean(
    ODFSetting, group, prefix + wxT("Percussive"), false, m_Percussive);
  m_OdfMidiKeyNumber = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("MIDIKeyNumber"), -1, 127, false, -1);
  m_OdfMidiPitchFraction = cfg.ReadFloat(
    ODFSetting,
    group,
    prefix + wxT("MIDIPitchFraction"),
    0,
    100.0,
    false,
    -1.0);
  m_LoopCrossfadeLength = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("LoopCrossfadeLength"), 0, 120, false, 0);
  m_ReleaseCrossfadeLength = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("ReleaseCrossfadeLength"),
    0,
    200,
    false,
    0);
  m_RetunePipe = cfg.ReadBoolean(
    ODFSetting, group, prefix + wxT("AcceptsRetuning"), false, m_RetunePipe);
  UpdateAmplitude();
  m_OrganController->GetWindchest(m_SamplerGroupID - 1)->AddPipe(this);

  LoadAttack(cfg, group, prefix);

  unsigned attack_count = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("AttackCount"), 0, 100, false, 0);
  for (unsigned i = 0; i < attack_count; i++)
    LoadAttack(cfg, group, prefix + wxString::Format(wxT("Attack%03d"), i + 1));

  unsigned release_count = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("ReleaseCount"), 0, 100, false, 0);
  for (unsigned i = 0; i < release_count; i++) {
    release_load_info rinfo;
    wxString p = prefix + wxString::Format(wxT("Release%03d"), i + 1);

    rinfo.filename.Assign(cfg.ReadFileName(ODFSetting, group, p));
    rinfo.sample_group = cfg.ReadInteger(
      ODFSetting, group, p + wxT("IsTremulant"), -1, 1, false, -1);
    rinfo.max_playback_time = cfg.ReadInteger(
      ODFSetting, group, p + wxT("MaxKeyPressTime"), -1, 100000, false, -1);
    rinfo.cue_point = cfg.ReadInteger(
      ODFSetting, group, p + wxT("CuePoint"), -1, MAX_SAMPLE_LENGTH, false, -1);
    rinfo.release_end = cfg.ReadInteger(
      ODFSetting,
      group,
      p + wxT("ReleaseEnd"),
      -1,
      MAX_SAMPLE_LENGTH,
      false,
      -1);

    m_ReleaseInfo.push_back(rinfo);
  }

  m_MinVolume = cfg.ReadFloat(
    ODFSetting, group, wxT("MinVelocityVolume"), 0, 1000, false, m_MinVolume);
  m_MaxVolume = cfg.ReadFloat(
    ODFSetting, group, wxT("MaxVelocityVolume"), 0, 1000, false, m_MaxVolume);
  m_SoundProvider.SetVelocityParameter(m_MinVolume, m_MaxVolume);
  m_PipeConfigNode.SetName(
    wxString::Format(_("%d: %s"), m_MidiKeyNumber, m_Filename.c_str()));
}

void GOSoundingPipe::LoadData(
  const GOFileStore &fileStore, GOMemoryPool &pool) {
  try {
    m_SoundProvider.LoadFromFile(
      fileStore,
      pool,
      m_AttackInfo,
      m_ReleaseInfo,
      m_PipeConfigNode.GetEffectiveBitsPerSample(),
      m_PipeConfigNode.GetEffectiveChannels(),
      m_PipeConfigNode.GetEffectiveCompress(),
      (loop_load_type)m_PipeConfigNode.GetEffectiveLoopLoad(),
      m_PipeConfigNode.GetEffectiveAttackLoad(),
      m_PipeConfigNode.GetEffectiveReleaseLoad(),
      m_LoopCrossfadeLength,
      m_ReleaseCrossfadeLength);
    Validate();
  } catch (wxString str) {
    m_SoundProvider.ClearData();
    throw wxString::Format(
      _("Error while loading samples for rank %s pipe %s: %s"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str(),
      str.c_str());
  } catch (std::bad_alloc &ba) {
    m_SoundProvider.ClearData();
    throw GOOutOfMemory();
  } catch (GOOutOfMemory e) {
    m_SoundProvider.ClearData();
    throw GOOutOfMemory();
  }
}

bool GOSoundingPipe::LoadCache(GOMemoryPool &pool, GOCache &cache) {
  try {
    bool result = m_SoundProvider.LoadCache(pool, cache);
    if (result)
      Validate();
    return result;
  } catch (std::bad_alloc &ba) {
    m_SoundProvider.ClearData();
    throw GOOutOfMemory();
  } catch (GOOutOfMemory e) {
    m_SoundProvider.ClearData();
    throw GOOutOfMemory();
  }
}

bool GOSoundingPipe::SaveCache(GOCacheWriter &cache) {
  return m_SoundProvider.SaveCache(cache);
}

void GOSoundingPipe::UpdateHash(GOHash &hash) {
  hash.Update(m_Filename);
  hash.Update(m_PipeConfigNode.GetEffectiveBitsPerSample());
  hash.Update(m_PipeConfigNode.GetEffectiveCompress());
  hash.Update(m_PipeConfigNode.GetEffectiveChannels());
  hash.Update(m_PipeConfigNode.GetEffectiveLoopLoad());
  hash.Update(m_PipeConfigNode.GetEffectiveAttackLoad());
  hash.Update(m_PipeConfigNode.GetEffectiveReleaseLoad());
  hash.Update(m_OdfMidiKeyNumber);
  hash.Update(m_LoopCrossfadeLength);
  hash.Update(m_ReleaseCrossfadeLength);

  hash.Update(m_AttackInfo.size());
  for (unsigned i = 0; i < m_AttackInfo.size(); i++) {
    m_AttackInfo[i].filename.Hash(hash);
    hash.Update(m_AttackInfo[i].sample_group);
    hash.Update(m_AttackInfo[i].max_playback_time);
    hash.Update(m_AttackInfo[i].load_release);
    hash.Update(m_AttackInfo[i].percussive);
    hash.Update(m_AttackInfo[i].cue_point);
    hash.Update(m_AttackInfo[i].loops.size());
    hash.Update(m_AttackInfo[i].attack_start);
    hash.Update(m_AttackInfo[i].release_end);
    for (unsigned j = 0; j < m_AttackInfo[i].loops.size(); j++) {
      hash.Update(m_AttackInfo[i].loops[j].m_StartPosition);
      hash.Update(m_AttackInfo[i].loops[j].m_EndPosition);
    }
  }

  hash.Update(m_ReleaseInfo.size());
  for (unsigned i = 0; i < m_ReleaseInfo.size(); i++) {
    m_ReleaseInfo[i].filename.Hash(hash);
    hash.Update(m_ReleaseInfo[i].sample_group);
    hash.Update(m_ReleaseInfo[i].max_playback_time);
    hash.Update(m_ReleaseInfo[i].cue_point);
    hash.Update(m_ReleaseInfo[i].release_end);
  }
}

float GOSoundingPipe::GetManualTuningPitchOffset() const {
  return m_PipeConfigNode.GetEffectivePitchTuning()
    + m_PipeConfigNode.GetEffectiveManualTuning();
}

float GOSoundingPipe::GetAutoTuningPitchOffset() const {
  float pitchAdjustment = 0.0;

  // For any other temperament than original. Calculate pitchAdjustment by
  // converting from the original temperament to the equal one before using
  // temperament offset. Take PitchCorrection into account. Also GUI tuning
  // adjustments are added and ODF adjustments removed leaving difference.
  if (!m_PipeConfigNode.GetEffectiveIgnorePitch() && m_SampleMidiKeyNumber) {
    pitchAdjustment
      = log(m_HarmonicNumber / 8.0) / log(2) * 1200 // harmonic correction
      + ((int)m_MidiKeyNumber - m_SampleMidiKeyNumber) * 100 // note correction
      - m_SampleMidiPitchFraction; // fraction correction
  }
  return pitchAdjustment + m_PipeConfigNode.GetEffectivePitchCorrection()
    + m_PipeConfigNode.GetEffectiveAutoTuningCorection(); // final correction
}

void GOSoundingPipe::Validate() {
  // make effective values
  m_SampleMidiKeyNumber = m_OdfMidiKeyNumber >= 0
    ? m_OdfMidiKeyNumber
    : m_SoundProvider.GetMidiKeyNumber();
  m_SampleMidiPitchFraction = m_OdfMidiPitchFraction >= 0.0
    ? m_OdfMidiPitchFraction
    : m_OdfMidiKeyNumber < 0
    ? m_SoundProvider.GetMidiPitchFract()
    : 0.0; // if MidiKeyNumber is provided in the ODF then we ignore
           // the PitchFraction from the sample

  if (!m_OrganController->GetConfig().ODFCheck())
    return;

  if (!m_PipeConfigNode.GetEffectiveChannels())
    return;

  if (m_SoundProvider.checkForMissingAttack()) {
    wxLogWarning(
      _("rank %s pipe %s: attack with MaxTimeSinceLastRelease=-1 missing"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
  }

  if (m_SoundProvider.checkForMissingRelease()) {
    wxLogWarning(
      _("rank %s pipe %s: default release is missing"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
  }

  if (m_SoundProvider.checkMissingRelease()) {
    wxLogWarning(
      _("rank %s pipe %s: no release defined"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
  }

  if (m_SoundProvider.checkNotNecessaryRelease()) {
    wxLogWarning(
      _("rank %s pipe %s: percussive sample with a release"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
  }

  if (
    m_RetunePipe && m_SampleMidiKeyNumber <= 0
    && m_SampleMidiPitchFraction <= 0.0) {
    wxLogWarning(
      _("rank %s pipe %s: no pitch information provided"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
    return;
  }
  float offset = m_RetunePipe ? GetAutoTuningPitchOffset() : 0.0;

  if (offset < -1800 || offset > 1800) {
    wxLogError(
      _("rank %s pipe %s: temperament would retune pipe by %f - more than "
        "1800 cent"),
      m_Rank->GetName(),
      GetLoadTitle(),
      offset);
    return;
  }
  if (offset < -600 || offset > 600) {
    wxLogWarning(
      _("rank %s pipe %s: temperament would retune pipe by more "
        "than 600 cent"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
    return;
  }
}

void GOSoundingPipe::SetTremulant(bool on) {
  if (on != m_Tremulant) {
    m_Tremulant = on;
    m_SoundProvider.UseSampleGroup((unsigned)on);
    if (m_Sampler)
      m_OrganController->SwitchSample(&m_SoundProvider, m_Sampler);
  }
}

void GOSoundingPipe::Change(unsigned velocity, unsigned last_velocity) {
  if (!m_Instances && velocity) {
    // the key pressed
    m_Sampler = m_OrganController->StartSample(
      &m_SoundProvider,
      m_SamplerGroupID,
      m_AudioGroupID,
      velocity,
      m_PipeConfigNode.GetEffectiveDelay(),
      m_LastStop);
    if (m_Sampler)
      m_Instances++;
    if (m_SoundProvider.IsOneshot())
      m_Sampler = nullptr;
  } else if (m_Instances && !velocity) {
    // the key released
    m_Instances--;
    if (m_Sampler) {
      m_LastStop = m_OrganController->StopSample(&m_SoundProvider, m_Sampler);
      m_Sampler = nullptr;
    }
  } else if (m_Sampler && last_velocity != velocity)
    // the key was pressed before and the velocity is changed now
    m_OrganController->UpdateVelocity(&m_SoundProvider, m_Sampler, velocity);
}

void GOSoundingPipe::UpdateAmplitude() {
  m_SoundProvider.SetAmplitude(
    m_PipeConfigNode.GetEffectiveAmplitude(),
    m_PipeConfigNode.GetEffectiveGain());
}

void GOSoundingPipe::UpdateTuning() {
  float pitchAdjustment = m_IsTemperamentOriginalBased
    ? GetManualTuningPitchOffset()
    : GetAutoTuningPitchOffset();

  m_SoundProvider.SetTuning(pitchAdjustment + m_TemperamentOffset);
}

void GOSoundingPipe::UpdateAudioGroup() {
  m_AudioGroupID = m_OrganController->GetConfig().GetAudioGroupId(
    m_PipeConfigNode.GetEffectiveAudioGroup());
}

void GOSoundingPipe::UpdateReleaseTail() {
  m_SoundProvider.SetReleaseTail(m_PipeConfigNode.GetEffectiveReleaseTail());
}

void GOSoundingPipe::SetTemperament(const GOTemperament &temperament) {
  m_IsTemperamentOriginalBased = temperament.IsTemperamentOriginalBased();
  if (!m_RetunePipe)
    m_TemperamentOffset = 0;
  else
    m_TemperamentOffset = temperament.GetOffset(m_MidiKeyNumber % 12);
  UpdateTuning();
}

void GOSoundingPipe::PreparePlayback() {
  GOPipe::PreparePlayback();
  UpdateAudioGroup();
}

void GOSoundingPipe::AbortPlayback() {
  m_Instances = 0;
  m_Tremulant = false;
  m_Sampler = 0;
  m_LastStop = 0;
  m_SoundProvider.UseSampleGroup(0);
  GOPipe::AbortPlayback();
}
