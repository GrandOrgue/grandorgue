/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROVIDER_H_
#define GOSOUNDPROVIDER_H_

#include <cstdint>
#include <vector>

#include "GOStatisticCallback.h"
#include "ptrvector.h"

class GOSoundAudioSection;
class GOCache;
class GOCacheWriter;
class GOHash;
class GOMemoryPool;

typedef struct audio_section_stream_s audio_section_stream;

class GOSoundProvider : public GOStatisticCallback {
protected:
  struct AttackSelector {
    unsigned min_attack_velocity;
    unsigned max_released_time;
    int8_t sample_group;
  };

  struct ReleaseSelector {
    unsigned max_playback_time;
    int8_t sample_group;
  };

  unsigned m_MidiKeyNumber;
  float m_MidiPitchFract;
  float m_Gain;
  float m_Tuning;
  bool m_SampleGroup;
  unsigned m_ReleaseTail;
  ptr_vector<GOSoundAudioSection> m_Attack;
  std::vector<AttackSelector> m_AttackInfo;
  ptr_vector<GOSoundAudioSection> m_Release;
  std::vector<ReleaseSelector> m_ReleaseInfo;
  void ComputeReleaseAlignmentInfo();
  float m_VelocityVolumeBase;
  float m_VelocityVolumeIncrement;
  unsigned m_AttackSwitchCrossfadeLength;

public:
  static void UpdateCacheHash(GOHash &hash);

  GOSoundProvider();
  virtual ~GOSoundProvider();

  void ClearData();

  virtual bool LoadCache(GOMemoryPool &pool, GOCache &cache);
  virtual bool SaveCache(GOCacheWriter &cache) const;

  void UseSampleGroup(bool sampleGroup) { m_SampleGroup = sampleGroup; }

  void SetVelocityParameter(float min_volume, float max_volume);

  const GOSoundAudioSection *GetAttack(
    unsigned velocity, unsigned releasedDurationMs) const;
  const GOSoundAudioSection *GetRelease(
    int8_t sampleGroup, unsigned playbackDurationMs) const;
  float GetGain() const;
  int IsOneshot() const;

  float GetTuning() const;
  void SetTuning(float cent);
  unsigned GetReleaseTail() const { return m_ReleaseTail; }
  void SetReleaseTail(unsigned releaseTail) { m_ReleaseTail = releaseTail; }

  unsigned GetMidiKeyNumber() const;
  float GetMidiPitchFract() const;
  unsigned GetAttackSwitchCrossfadeLength() const {
    return m_AttackSwitchCrossfadeLength;
  }

  float GetVelocityVolume(unsigned velocity) const;

  bool checkForMissingAttack();
  bool checkForMissingRelease();
  bool checkMissingRelease();
  bool checkNotNecessaryRelease();

  GOSampleStatistic GetStatistic();
};

inline float GOSoundProvider::GetGain() const { return m_Gain; }

inline float GOSoundProvider::GetTuning() const { return m_Tuning; }

#endif /* GOSOUNDPROVIDER_H_ */
