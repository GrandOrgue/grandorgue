/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSAMPLER_H_
#define GOSOUNDSAMPLER_H_

#include "GOSoundAudioSection.h"
#include "GOSoundFader.h"

class GOSoundProvider;
class GOSoundWindchestWorkItem;

struct GOSoundSampler {
  GOSoundSampler *next;
  const GOSoundProvider *p_SoundProvider;
  int m_SamplerTaskId;
  GOSoundWindchestWorkItem *p_WindchestTask;
  unsigned m_AudioGroupId;
  audio_section_stream stream;
  GOSoundFader fader;
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
