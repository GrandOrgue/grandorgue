/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTTASK_H
#define GOSOUNDTREMULANTTASK_H

#include "sound/GOSoundSamplerList.h"
#include "sound/scheduler/GOSoundTask.h"
#include "threading/GOMutex.h"

class GOSoundEngine;

class GOSoundTremulantTask : public GOSoundTask {
private:
  GOSoundEngine &m_engine;
  GOSoundSamplerList m_Samplers;
  GOMutex m_Mutex;
  float m_Volume;
  unsigned m_SamplesPerBuffer;
  bool m_Done;

public:
  GOSoundTremulantTask(
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
