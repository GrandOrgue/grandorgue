/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTWORKITEM_H
#define GOSOUNDTREMULANTWORKITEM_H

#include "sound/GOSoundSamplerList.h"
#include "sound/scheduler/GOSoundWorkItem.h"
#include "threading/GOMutex.h"

class GOSoundEngine;

class GOSoundTremulantWorkItem : public GOSoundWorkItem {
private:
  GOSoundEngine &m_engine;
  GOSoundSamplerList m_Samplers;
  GOMutex m_Mutex;
  float m_Volume;
  unsigned m_SamplesPerBuffer;
  bool m_Done;

public:
  GOSoundTremulantWorkItem(
    GOSoundEngine &sound_engine, unsigned samples_per_buffer);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *thread = nullptr);
  void Exec();

  void Reset();
  void Clear();
  void Add(GOSoundSampler *sampler);

  float GetVolume() {
    if (!m_Done)
      Run();
    return m_Volume;
  }
};

#endif
