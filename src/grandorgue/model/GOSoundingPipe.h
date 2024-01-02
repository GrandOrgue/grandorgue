/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDINGPIPE_H
#define GOSOUNDINGPIPE_H

#include "pipe-config/GOPipeConfigNode.h"
#include "sound/GOSoundProviderWave.h"

#include "GOCacheObject.h"
#include "GOPipe.h"
#include "GOPipeWindchestCallback.h"

class GOMemoryPool;
class GOOrganModel;
class GOSoundSampler;

class GOSoundingPipe : public GOPipe,
                       private GOCacheObject,
                       private GOPipeUpdateCallback,
                       private GOPipeWindchestCallback {
private:
  GOOrganModel *p_OrganModel;
  GOSoundSampler *m_Sampler;
  uint64_t m_LastStop;
  int m_Instances;
  bool m_Tremulant;
  std::vector<GOSoundProviderWave::AttackFileInfo> m_AttackInfo;
  std::vector<GOSoundProviderWave::ReleaseFileInfo> m_ReleaseInfo;
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
  float m_MinVolume;
  float m_MaxVolume;
  int m_OdfMidiKeyNumber;
  float m_OdfMidiPitchFraction;
  int m_SampleMidiKeyNumber;
  float m_SampleMidiPitchFraction;
  bool m_RetunePipe;
  bool m_IsTemperamentOriginalBased;
  GOSoundProviderWave m_SoundProvider;
  GOPipeConfigNode m_PipeConfigNode;

  // internal functions
  void LoadAttack(GOConfigReader &cfg, wxString group, wxString prefix);
  /**
   * Calculate a pitch offset for manual tuning
   * @return pitch offset in cents
   */
  float GetManualTuningPitchOffset() const;
  /**
   * Calculates a pitch offset for retuning from the sample pitch
   * to the equal temperament
   * @return pitch offset in cents
   */
  float GetAutoTuningPitchOffset() const;
  void Validate();

  // Callbacks for GOCacheObject
  const wxString &GetLoadTitle() const override { return m_Filename; }
  void Initialize() override {}
  void LoadData(const GOFileStore &fileStore, GOMemoryPool &pool) override;
  bool LoadCache(GOMemoryPool &pool, GOCache &cache) override;
  bool SaveCache(GOCacheWriter &cache) const override;
  void UpdateHash(GOHash &hash) const override;

  // Callbacks from GOPipeConfigNode
  void UpdateAmplitude() override;
  void UpdateTuning() override;
  void UpdateAudioGroup() override;
  void UpdateReleaseTail() override;
  void SetTemperament(const GOTemperament &temperament) override;

  // Callbacks from the sound engine
  void PreparePlayback() override;
  void AbortPlayback() override;

  // Callbacks from the console
  /**
   * Called when the tremulant is switched on or off
   * @param on the new tremulant state
   */
  void SetTremulant(bool on) override;
  /**
   * Called when the key is just pressed, released or the velocity is changed
   * @param velocity the velocity of key pressing. 0 means release
   * @param last_velocity the velocity of the previous key pressing
   */
  void VelocityChanged(unsigned velocity, unsigned old_velocity) override;

public:
  GOSoundingPipe(
    GOOrganModel *organController,
    GORank *rank,
    bool percussive,
    int sampler_group_id,
    unsigned midi_key_number,
    unsigned harmonic_number,
    float min_volume,
    float max_volume,
    bool retune);

  void Init(
    GOConfigReader &cfg, wxString group, wxString prefix, wxString filename);
  void Load(GOConfigReader &cfg, wxString group, wxString prefix);
};

#endif
