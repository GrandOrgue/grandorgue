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
#include "GOSoundFader.h"
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
  GOSoundFader fader;
  GOSoundToneBalanceFilter::State toneBalanceFilterState;
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
