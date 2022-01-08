/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDINGPIPE_H
#define GOSOUNDINGPIPE_H

#include "GOCacheObject.h"
#include "GOPipe.h"
#include "GOPipeConfigNode.h"
#include "GOPipeWindchestCallback.h"
#include "sound/GOSoundProviderWave.h"

class GOSoundSampler;

class GOSoundingPipe : public GOPipe,
                       private GOCacheObject,
                       private GOPipeUpdateCallback,
                       private GOPipeWindchestCallback {
 private:
  GOSoundSampler *m_Sampler;
  uint64_t m_LastStop;
  int m_Instances;
  bool m_Tremulant;
  std::vector<attack_load_info> m_AttackInfo;
  std::vector<release_load_info> m_ReleaseInfo;
  wxString m_Filename;

  /* states which windchest this pipe belongs to, see
   * GOSoundEngine::StartSampler */
  int m_SamplerGroupID;
  unsigned m_AudioGroupID;
  bool m_Percussive;
  float m_TemperamentOffset;
  unsigned m_HarmonicNumber;
  unsigned m_LoopCrossfadeLength;
  unsigned m_ReleaseCrossfadeLength;
  float m_PitchCorrection;
  float m_MinVolume;
  float m_MaxVolume;
  int m_SampleMidiKeyNumber;
  bool m_RetunePipe;
  GOSoundProviderWave m_SoundProvider;
  GOPipeConfigNode m_PipeConfig;

  void SetOn(unsigned velocity);
  void SetOff();
  void Change(unsigned velocity, unsigned old_velocity);
  GOSoundProvider *GetSoundProvider();
  void Validate();

  void LoadAttack(GOConfigReader &cfg, wxString group, wxString prefix);

  void Initialize();
  void LoadData();
  bool LoadCache(GOCache &cache);
  bool SaveCache(GOCacheWriter &cache);
  void UpdateHash(GOHash &hash);
  const wxString &GetLoadTitle();

  void SetTremulant(bool on);

  void UpdateAmplitude();
  void UpdateTuning();
  void UpdateAudioGroup();

  void AbortPlayback();
  void PreparePlayback();

 public:
  GOSoundingPipe(
    GODefinitionFile *organfile,
    GORank *rank,
    bool percussive,
    int sampler_group_id,
    unsigned midi_key_number,
    unsigned harmonic_number,
    float pitch_correction,
    float min_volume,
    float max_volume,
    bool retune);

  void Init(
    GOConfigReader &cfg, wxString group, wxString prefix, wxString filename);
  void Load(GOConfigReader &cfg, wxString group, wxString prefix);
  void SetTemperament(const GOTemperament &temperament);
};

#endif
