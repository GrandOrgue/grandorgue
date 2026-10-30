/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSAMPLER_H_
#define GOSOUNDSAMPLER_H_

#include "sound/playing/GOSoundToneBalanceFilter.h"

#include "GOBool3.h"
#include "GOSoundPlayingFader.h"
#include "GOSoundStream.h"

class GOSoundProvider;
class GOSoundWindchestTask;

struct GOSoundSampler {
  GOSoundSampler *next;
  const GOSoundProvider *p_SoundProvider;
  int m_SamplerTaskId;
  /** Selects the GOSoundWindchestGroupTask grid cell this sampler mixes
   * into, together with m_AudioGroupId - see GOSoundSamplerPlayer::
   * PassSampler(). Equal to m_SamplerTaskId for every sampler except a
   * detached release tail (CreateReleaseSampler()), where it keeps the
   * true originating windchest (so the tail still runs through that
   * windchest's chain) while m_SamplerTaskId becomes
   * DETACHED_RELEASE_TASK_ID (the volume source for the tail's fader). */
  int m_MixWindchestTaskId;
  GOSoundWindchestTask *p_WindchestTask;
  unsigned m_AudioGroupId;
  GOSoundStream stream;
  GOSoundPlayingFader fader;
  GOSoundToneBalanceFilter::State toneBalanceFilterState;
  /** The provider gain (GOSoundProvider::GetGain()) that is baked into the
   * fader's target volume. When the provider's gain is changed during playback,
   * the sampler applies the ratio of the new gain to this value - see
   * GOSoundSamplerPlayer::ApplyLiveProviderChanges(). */
  float m_TargetGain;
  /** The provider tuning (GOSoundProvider::GetTuning()) at which
   * m_TargetResamplerIncrement was computed */
  float m_TargetTuning;
  /** The resampling increment (see GOSoundResample::rateToFractionIncrement())
   * of the stream at the provider tuning m_TargetTuning. When the provider's
   * tuning is changed during playback, the increment for the new tuning is
   * scaled from this value, not from the previous result, so the rounding
   * errors do not accumulate. */
  unsigned m_TargetResamplerIncrement;
  /** The resampling increment that is currently passed to
   * GOSoundStream::ReadBlock(). It differs from the increment for the current
   * provider tuning only until the next audio buffer. */
  unsigned m_AppliedResamplerIncrement;

  /**
   * Sets the resampling increment that corresponds to the provider tuning. The
   * stream will be read with it, and later tuning changes are scaled from it.
   * @param tuning the provider tuning (GOSoundProvider::GetTuning()) the
   *   increment was computed for; must be positive
   * @param resamplerIncrement the increment of the stream at this tuning
   */
  void InitResamplerIncrement(float tuning, unsigned resamplerIncrement);

  /**
   * @param tuning the provider tuning; must be positive
   * @return the resampling increment of the stream for the given provider
   *   tuning, limited as GOSoundResample::scaleFractionIncrement() does
   */
  unsigned GetResamplerIncrementFor(float tuning) const;

  uint64_t time;
  unsigned velocity;
  unsigned delay;
  /* current index of the current block into this sample */
  volatile unsigned long stop;
  volatile unsigned long new_attack;
  GOBool3 m_WaveTremulantStateFor;
  bool is_release;
  unsigned drop_counter;
};

#endif /* GOSOUNDSAMPLER_H_ */
