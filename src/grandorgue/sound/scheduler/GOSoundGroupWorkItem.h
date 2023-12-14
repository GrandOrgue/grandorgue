/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDGROUPWORKITEM_H
#define GOSOUNDGROUPWORKITEM_H

#include <atomic>

#include "GOSoundThread.h"
#include "sound/GOSoundBufferItem.h"
#include "sound/GOSoundSamplerList.h"
#include "sound/scheduler/GOSoundWorkItem.h"
#include "threading/GOCondition.h"
#include "threading/GOMutex.h"

class GOSoundEngine;

class GOSoundGroupWorkItem : public GOSoundWorkItem, public GOSoundBufferItem {
private:
  GOSoundEngine &m_engine;
  GOSoundSamplerList m_Active;
  GOSoundSamplerList m_Release;
  GOMutex m_Mutex;
  GOCondition m_Condition;

  // the number of threads are processing the samples
  std::atomic_uint m_ActiveCount;

  // processing state
  //   0 - no threads are processing the samples
  //   1 - some threads are processing the samples and none has finished
  //   2 - some thread has finished processing samples but not all
  //   3 - all threads have finished processing samples
  std::atomic_uint m_Done;
  std::atomic_bool m_Stop;

  void ProcessList(
    GOSoundSamplerList &list, bool toDropOld, float *output_buffer);

public:
  GOSoundGroupWorkItem(
    GOSoundEngine &sound_engine, unsigned samples_per_buffer);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *pThread = nullptr);
  void Exec();
  void Finish(bool stop, GOSoundThread *pThread = nullptr);

  void Reset();
  void Clear();
  void Add(GOSoundSampler *sampler);
  void WaitAndClear();
};

#endif
