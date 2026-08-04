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

#include "GOSoundTaskBase.h"
#include "ptrvector.h"

class GOSchedulerThread;
class GOSoundGroupTask;
class GOSoundSampler;
class GOSoundSamplerPlayer;

class GOSoundReleaseTask : public GOSoundTaskBase {
private:
  GOSoundSamplerPlayer &r_SamplerPlayer;
  ptr_vector<GOSoundGroupTask> &m_AudioGroups;
  GOSoundSimpleSamplerList m_List;
  std::atomic_uint m_WaitCnt;
  std::atomic_uint m_Cnt;
  std::atomic_bool m_IsToComplete;

public:
  GOSoundReleaseTask(
    GOSoundSamplerPlayer &samplerPlayer,
    ptr_vector<GOSoundGroupTask> &audioGroupTaskPtrs);

  unsigned GetPriority() const override { return PRIORITY_RELEASE; }
  unsigned GetCost() const override { return 0; }
  bool IsRepeatable() const override { return true; }
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void CompleteRound() override;

  void DiscardContent() override { m_List.Clear(); }
  void NewRound() override;

  void Add(GOSoundSampler *sampler);
};

#endif
