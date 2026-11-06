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
  /*
   * Every pipe parameter that can be changed while the sampler is playing (for
   * example, by the voicing controls) is kept in three parts:
   * - the initial value: fixed when the sampler starts playing a section (at
   *   the start, at an attack switch and at the release). It is the base that
   *   the fader and the stream were set up with;
   * - the last applied provider value: the value of the provider the sampler
   *   has followed last;
   * - the derived value: what the audio code actually uses. It is always
   *   computed from the initial value, never from the previously derived one,
   *   so the rounding errors do not accumulate.
   *
   * An Ensure...() method is called once per audio buffer before the buffer is
   * read. It compares the provider's current value with the last applied one
   * and recomputes the derived value only if they differ. An Init...() method
   * sets all three parts together.
   */

  /** The provider's volume factor (GOSoundProvider::GetGain()) that is baked
   * into the fader's target volume when the section started */
  float m_InitialVolumeFactor;
  /** The provider's volume factor the sampler has followed last */
  float m_AppliedVolumeFactor;
  /** The ratio of m_AppliedVolumeFactor to m_InitialVolumeFactor. The caller
   * passes it to the fader, which changes the volume smoothly. 1.0 if the
   * provider's volume factor was not changed */
  float m_AppliedVolumeRatio;

  /** The provider's tuning factor (GOSoundProvider::GetTuning()) at which
   * m_InitialResamplerIncrement was computed */
  float m_InitialTuningFactor;
  /** The resampling increment of the stream at the tuning factor
   * m_InitialTuningFactor */
  unsigned m_InitialResamplerIncrement;
  /** The provider's tuning factor the sampler has followed last */
  float m_AppliedTuningFactor;
  /** The resampling increment that is passed to GOSoundStream::ReadBlock():
   * m_InitialResamplerIncrement scaled by the ratio of m_AppliedTuningFactor to
   * m_InitialTuningFactor */
  unsigned m_AppliedResamplerIncrement;

  /**
   * Starts following the provider's volume factor from the given value.
   * @param volumeFactor the provider's volume factor
   *   (GOSoundProvider::GetGain()) that the fader of the new section is set up
   *   with
   */
  void InitVolumeFactor(float volumeFactor);

  /**
   * Makes the sampler follow the provider's volume factor. Idempotent: does
   * nothing if the factor is the same as at the previous call.
   * @param volumeFactor the provider's current volume factor
   */
  void EnsureVolumeFactor(float volumeFactor);

  /**
   * Starts following the provider's tuning factor from the given value.
   * @param tuningFactor the provider's tuning factor
   *   (GOSoundProvider::GetTuning()) the increment was computed for; must be
   *   positive
   * @param resamplerIncrement the increment of the stream at this tuning
   *   factor
   */
  void InitResamplerIncrement(float tuningFactor, unsigned resamplerIncrement);

  /**
   * Makes the resampling increment follow the provider's tuning factor,
   * keeping the stream position. Idempotent: does nothing if the factor is the
   * same as at the previous call.
   * @param tuningFactor the provider's current tuning factor; must be positive
   */
  void EnsureTuningFactor(float tuningFactor);

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
