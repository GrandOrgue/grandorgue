/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDRELEASETASK_H
#define GOSOUNDRELEASETASK_H

#include <atomic>

#include "sound/playing/GOSoundSimpleSamplerList.h"
#include "sound/scheduler/GOSoundTask.h"

#include "ptrvector.h"

class GOSoundGroupTask;
class GOSoundSampler;
class GOSoundSamplerPlayer;

class GOSoundReleaseTask : public GOSoundTask {
private:
  GOSoundSamplerPlayer &r_SamplerPlayer;
  ptr_vector<GOSoundGroupTask> &m_AudioGroups;
  GOSoundSimpleSamplerList m_List;
  std::atomic_uint m_WaitCnt;
  std::atomic_uint m_Cnt;
  std::atomic_bool m_Stop;

public:
  GOSoundReleaseTask(
    GOSoundSamplerPlayer &samplerPlayer,
    ptr_vector<GOSoundGroupTask> &audioGroupTaskPtrs);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *thread = nullptr);
  void Exec();

  void Clear();
  void Reset();

  void Add(GOSoundSampler *sampler);
};

#endif
