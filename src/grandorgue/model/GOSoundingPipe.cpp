/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundingPipe.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "sound/GOSoundEngine.h"
#include "temperaments/GOTemperament.h"

#include "GOAlloc.h"
#include "GOHash.h"
#include "GOOrganModel.h"
#include "GOPath.h"
#include "GORank.h"
#include "GOWindchest.h"

#include "go_limits.h"

GOSoundingPipe::GOSoundingPipe(
  GOOrganModel *pOrganModel,
  GORank *rank,
  unsigned windchestN,
  unsigned midi_key_number,
  unsigned harmonic_number,
  float min_volume,
  float max_volume,
  bool retune)
  : GOPipe(pOrganModel, rank, midi_key_number),
    p_OrganModel(pOrganModel),
    p_CurrentLoopSampler(nullptr),
    m_LastStart(0),
    m_LastStop(0),
    m_Instances(0),
    m_AttackFileInfos(),
    m_ReleaseFileInfos(),
    m_Filename(),
    m_WindchestN(windchestN),
    m_AudioGroupID(0),
    m_TemperamentOffset(0),
    m_HarmonicNumber(harmonic_number),
    m_MinVolume(min_volume),
    m_MaxVolume(max_volume),
    m_OdfMidiKeyNumber(-1),
    m_OdfMidiPitchFraction(-1.0),
    m_SampleMidiKeyNumber(0),
    m_SampleMidiPitchFraction(0.0),
    m_RetunePipe(retune),
    m_IsTemperamentOriginalBased(true),
    m_SoundProvider(this),
    m_PipeConfigNode(
      &rank->GetPipeConfig(), *pOrganModel, this, &m_SoundProvider) {}

void GOSoundingPipe::Init(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &prefix,
  const wxString &filename) {
  SetGroupAndPrefix(group, prefix);
  p_OrganModel->RegisterCacheObject(this);
  m_Filename = filename;
  m_PipeConfigNode.Init(cfg, group, prefix);
  m_OdfMidiKeyNumber = -1;
  m_OdfMidiPitchFraction = -1.0;
  UpdateAmplitude();
  p_OrganModel->GetWindchest(m_WindchestN - 1)->AddPipe(this);

  GOSoundProviderWave::AttackFileInfo ainfo;

  ainfo.filename.AssignResource(m_Filename);
  ainfo.m_WaveTremulantStateFor = BOOL3_DEFAULT;
  ainfo.percussive = m_PipeConfigNode.GetEffectivePercussive();
  ainfo.load_release = !ainfo.percussive;
  ainfo.max_playback_time = -1;
  ainfo.cue_point = -1;
  ainfo.min_attack_velocity = 0;
  ainfo.max_released_time = -1;
  ainfo.attack_start = 0;
  ainfo.release_end = -1;
  ainfo.m_LoopCrossfadeLength = 0;
  ainfo.m_ReleaseCrossfadeLength = 0;
  m_AttackFileInfos.push_back(ainfo);

  m_SoundProvider.SetVelocityParameter(m_MinVolume, m_MaxVolume);
  m_PipeConfigNode.SetName(
    wxString::Format(_("%d: %s"), m_MidiKeyNumber, m_Filename.c_str()));
}

void GOSoundingPipe::LoadAttackFileInfo(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &prefix,
  bool isMainForSecondaryAttacks) {
  GOSoundProviderWave::AttackFileInfo ainfo;

  ainfo.filename.Assign(cfg.ReadFileName(ODFSetting, group, prefix));
  ainfo.m_WaveTremulantStateFor = cfg.ReadBool3FromInt(
    ODFSetting, group, prefix + wxT("IsTremulant"), false);
  ainfo.percussive = m_PipeConfigNode.GetEffectivePercussive();
  ainfo.load_release = cfg.ReadBoolean(
    ODFSetting, group, prefix + wxT("LoadRelease"), false, !ainfo.percussive);
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
  ainfo.m_LoopCrossfadeLength = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("LoopCrossfadeLength"), 0, 3000, false, 0);
  ainfo.m_ReleaseCrossfadeLength
    = isMainForSecondaryAttacks || ainfo.load_release ? cfg.ReadInteger(
        ODFSetting,
        group,
        prefix + wxT("ReleaseCrossfadeLength"),
        0,
        3000,
        false,
        0)
                                                      : 0;

  m_AttackFileInfos.push_back(ainfo);
}

void GOSoundingPipe::LoadReleaseFileInfo(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  GOSoundProviderWave::ReleaseFileInfo rinfo;

  rinfo.filename.Assign(cfg.ReadFileName(ODFSetting, group, prefix));
  rinfo.m_WaveTremulantStateFor = cfg.ReadBool3FromInt(
    ODFSetting, group, prefix + wxT("IsTremulant"), false);
  rinfo.max_playback_time = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("MaxKeyPressTime"), -1, 100000, false, -1);
  rinfo.cue_point = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("CuePoint"),
    -1,
    MAX_SAMPLE_LENGTH,
    false,
    -1);
  rinfo.release_end = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("ReleaseEnd"),
    -1,
    MAX_SAMPLE_LENGTH,
    false,
    -1);
  rinfo.m_ReleaseCrossfadeLength = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("ReleaseCrossfadeLength"),
    0,
    3000,
    false,
    0);
  m_ReleaseFileInfos.push_back(rinfo);
}

void GOSoundingPipe::Load(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  SetGroupAndPrefix(group, prefix);
  p_OrganModel->RegisterCacheObject(this);
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
  m_WindchestN = cfg.ReadInteger(
    ODFSetting,
    group,
    prefix + wxT("WindchestGroup"),
    1,
    p_OrganModel->GetWindchestCount(),
    false,
    m_WindchestN);
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
  m_RetunePipe = cfg.ReadBoolean(
    ODFSetting, group, prefix + wxT("AcceptsRetuning"), false, m_RetunePipe);
  UpdateAmplitude();
  p_OrganModel->GetWindchest(m_WindchestN - 1)->AddPipe(this);

  unsigned attack_count = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("AttackCount"), 0, 100, false, 0);

  LoadAttackFileInfo(cfg, group, prefix, attack_count > 0);

  for (unsigned i = 1; i <= attack_count; i++)
    LoadAttackFileInfo(
      cfg, group, wxString::Format(wxT("%sAttack%03d"), prefix, i), false);

  unsigned release_count = cfg.ReadInteger(
    ODFSetting, group, prefix + wxT("ReleaseCount"), 0, 100, false, 0);

  if (m_PipeConfigNode.IsEffectiveIndependentRelease() && !release_count)
    wxLogWarning(
      _("The pipe %s/%s has independent release but %sReleaseCount=0"),
      group,
      prefix,
      prefix);

  for (unsigned i = 1; i <= release_count; i++)
    LoadReleaseFileInfo(
      cfg, group, wxString::Format(wxT("%sRelease%03d"), prefix, i));

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
    m_SoundProvider.LoadFromMultipleFiles(
      fileStore,
      pool,
      m_AttackFileInfos,
      m_ReleaseFileInfos,
      m_PipeConfigNode.GetEffectiveBitsPerSample(),
      m_PipeConfigNode.GetEffectiveChannels(),
      m_PipeConfigNode.GetEffectiveCompress(),
      (GOSoundProviderWave::LoopLoadType)
        m_PipeConfigNode.GetEffectiveLoopLoad(),
      m_PipeConfigNode.GetEffectiveAttackLoad(),
      m_PipeConfigNode.GetEffectiveReleaseLoad());
    Validate();
  } catch (std::bad_alloc &ba) {
    m_SoundProvider.ClearData();
    throw GOOutOfMemory();
  } catch (...) {
    m_SoundProvider.ClearData();
    throw;
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

bool GOSoundingPipe::SaveCache(GOCacheWriter &cache) const {
  return m_SoundProvider.SaveCache(cache);
}

void GOSoundingPipe::UpdateHash(GOHash &hash) const {
  hash.Update(m_Filename);
  hash.Update(m_PipeConfigNode.GetEffectiveBitsPerSample());
  hash.Update(m_PipeConfigNode.GetEffectiveCompress());
  hash.Update(m_PipeConfigNode.GetEffectiveChannels());
  hash.Update(m_PipeConfigNode.GetEffectiveLoopLoad());
  hash.Update(m_PipeConfigNode.GetEffectiveAttackLoad());
  hash.Update(m_PipeConfigNode.GetEffectiveReleaseLoad());
  hash.Update(m_OdfMidiKeyNumber);
  hash.Update(m_PipeConfigNode.IsEffectiveIndependentRelease());

  hash.Update(m_AttackFileInfos.size());
  for (const auto &a : m_AttackFileInfos) {
    a.filename.Hash(hash);
    hash.Update(a.m_WaveTremulantStateFor);
    hash.Update(a.max_playback_time);
    hash.Update(a.load_release);
    hash.Update(a.percussive);
    hash.Update(a.cue_point);
    hash.Update(a.loops.size());
    hash.Update(a.attack_start);
    hash.Update(a.release_end);
    for (unsigned j = 0; j < a.loops.size(); j++) {
      hash.Update(a.loops[j].m_StartPosition);
      hash.Update(a.loops[j].m_EndPosition);
    }
    hash.Update(a.m_LoopCrossfadeLength);
    hash.Update(a.m_ReleaseCrossfadeLength);
  }

  hash.Update(m_ReleaseFileInfos.size());
  for (const auto &r : m_ReleaseFileInfos) {
    r.filename.Hash(hash);
    hash.Update(r.m_WaveTremulantStateFor);
    hash.Update(r.max_playback_time);
    hash.Update(r.cue_point);
    hash.Update(r.release_end);
    hash.Update(r.m_ReleaseCrossfadeLength);
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

  if (!p_OrganModel->GetConfig().ODFCheck())
    return;

  if (!m_PipeConfigNode.GetEffectiveChannels())
    return;

  if (m_SoundProvider.CheckForMissingAttack()) {
    wxLogWarning(
      _("rank %s pipe %s: attack with MaxTimeSinceLastRelease=-1 missing"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
  }

  if (m_SoundProvider.CheckForMissingRelease()) {
    wxLogWarning(
      _("rank %s pipe %s: default release is missing"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
  }

  if (m_SoundProvider.CheckMissingRelease()) {
    wxLogWarning(
      _("rank %s pipe %s: no release defined"),
      m_Rank->GetName().c_str(),
      GetLoadTitle().c_str());
  }

  if (
    !m_PipeConfigNode.IsEffectiveIndependentRelease()
    && m_SoundProvider.CheckNotNecessaryRelease()) {
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
  float offset = m_RetunePipe
    ? (GetAutoTuningPitchOffset() - GetManualTuningPitchOffset())
    : 0.0;

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

void GOSoundingPipe::SetWaveTremulant(bool on) {
  if (m_SoundProvider.IsWaveTremulant() != on) {
    m_SoundProvider.SetWaveTremulant(on);

    GOSoundEngine *pSoundEngine = GetSoundEngine();

    if (
      pSoundEngine && p_CurrentLoopSampler
      && !m_SoundProvider.IsWaveTremulantStateSuitable(
        p_CurrentLoopSampler->m_WaveTremulantStateFor))
      pSoundEngine->SwitchSample(&m_SoundProvider, p_CurrentLoopSampler);
  }
}

void GOSoundingPipe::VelocityChanged(
  unsigned velocity, unsigned last_velocity) {
  GOSoundEngine *pSoundEngine = GetSoundEngine();

  if (!m_Instances && velocity) {
    // the key pressed
    GOSoundSampler *pSampler = pSoundEngine ? pSoundEngine->StartPipeSample(
                                 &m_SoundProvider,
                                 m_WindchestN,
                                 m_AudioGroupID,
                                 velocity,
                                 m_PipeConfigNode.GetEffectiveDelay(),
                                 m_LastStop,
                                 false,
                                 &m_LastStart)
                                            : nullptr;
    if (pSampler) {
      m_Instances++;
      if (!m_SoundProvider.IsOneshot()) {
        p_CurrentLoopSampler = pSampler;
      }
    }
  } else if (m_Instances && !velocity) {
    // the key released
    m_Instances--;
    if (p_CurrentLoopSampler) {
      m_LastStop = pSoundEngine
        ? pSoundEngine->StopSample(&m_SoundProvider, p_CurrentLoopSampler)
        : 0;
      p_CurrentLoopSampler = nullptr;
    } else if (m_PipeConfigNode.IsEffectiveIndependentRelease() && pSoundEngine)
      pSoundEngine->StartPipeSample(
        &m_SoundProvider,
        m_WindchestN,
        m_AudioGroupID,
        last_velocity,
        m_PipeConfigNode.GetEffectiveDelay(),
        m_LastStart,
        true,
        &m_LastStop);
  } else if (p_CurrentLoopSampler && last_velocity != velocity)
    // the key was pressed before and the velocity is changed now
    if (pSoundEngine)
      pSoundEngine->UpdateVelocity(
        &m_SoundProvider, p_CurrentLoopSampler, velocity);
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
  m_AudioGroupID = p_OrganModel->GetConfig().GetAudioGroupId(
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
  p_CurrentLoopSampler = 0;
  m_LastStop = 0;
  m_SoundProvider.SetWaveTremulant(false);
  GOPipe::AbortPlayback();
}
