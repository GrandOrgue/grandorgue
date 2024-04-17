/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDINGPIPE_H
#define GOSOUNDINGPIPE_H

#include "pipe-config/GOPipeConfigNode.h"
#include "pipe-config/GOPipeUpdateCallback.h"
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
  GOSoundSampler *p_CurrentLoopSampler;
  uint64_t m_LastStart;
  uint64_t m_LastStop;
  int m_Instances;
  std::vector<GOSoundProviderWave::AttackFileInfo> m_AttackFileInfos;
  std::vector<GOSoundProviderWave::ReleaseFileInfo> m_ReleaseFileInfos;
  wxString m_Filename;

  /* states which windchest this pipe belongs to, see
   * GOSoundEngine::StartSampler */
  unsigned m_WindchestN; // starts with 1
  unsigned m_AudioGroupID;
  float m_TemperamentOffset;
  unsigned m_HarmonicNumber;
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
  /* Read one attack file info from the odf keys with the prefix specified and
   * add it to m_AttackFileInfos
   */
  void LoadAttackFileInfo(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &prefix,
    bool isMain);
  /* Read one release file info from the odf keys with the prefix specified and
   * add it to m_AttackFileInfos
   */
  void LoadReleaseFileInfo(
    GOConfigReader &cfg, const wxString &group, const wxString &prefix);
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
   * Called when a wave tremulant is switched on or off
   * @param on the new tremulant state
   */
  void SetWaveTremulant(bool on) override;
  /**
   * Called when the key is just pressed, released or the velocity is changed
   * @param velocity the velocity of key pressing. 0 means release
   * @param last_velocity the velocity of the previous key pressing
   */
  void VelocityChanged(unsigned velocity, unsigned old_velocity) override;

public:
  GOSoundingPipe(
    GOOrganModel *pOrganModel,
    GORank *rank,
    unsigned windchestN,
    unsigned midi_key_number,
    unsigned harmonic_number,
    float min_volume,
    float max_volume,
    bool retune);

  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &prefix,
    const wxString &filename);
  void Load(GOConfigReader &cfg, const wxString &group, const wxString &prefix)
    override;
};

#endif
