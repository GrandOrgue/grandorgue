/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSAMPLER_H_
#define GOSOUNDSAMPLER_H_

#include "GOSoundAudioSection.h"
#include "GOSoundFader.h"

class GOSoundProvider;
class GOSoundWindchestWorkItem;

class GOSoundSampler {
public:
  GOSoundSampler *next;
  const GOSoundProvider *pipe;
  int sampler_group_id;
  GOSoundWindchestWorkItem *windchest;
  unsigned audio_group_id;
  audio_section_stream stream;
  GOSoundFader fader;
  uint64_t time;
  unsigned velocity;
  unsigned delay;
  /* current index of the current block into this sample */
  volatile unsigned long stop;
  volatile unsigned long new_attack;
  bool is_release;
  unsigned drop_counter;
};

#endif /* GOSOUNDSAMPLER_H_ */
