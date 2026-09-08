/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDGROUPTASK_H
#define GOSOUNDGROUPTASK_H

#include <atomic>

#include "sound/playing/GOSoundSamplerList.h"
#include "threading/GOCondition.h"

#include "GOSoundBufferTaskBase.h"

class GOSchedulerThread;
class GOSoundBufferMutable;
class GOSoundSamplerPlayer;

class GOSoundGroupTask : public GOSoundBufferTaskBase {
private:
  GOSoundSamplerPlayer &r_SamplerPlayer;
  GOSoundSamplerList m_Active;
  GOSoundSamplerList m_Release;
  GOCondition m_Condition;

  // the number of threads that are processing the samples
  std::atomic_uint m_ActiveCount;

  void ProcessList(
    GOSoundSamplerList &list,
    bool isToDropOld,
    GOSoundBufferMutable &outBuffer);
  void DoNewRound() override { m_ActiveCount.store(0); }

public:
  GOSoundGroupTask(
    GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer);

  unsigned GetCost() const override;
  bool IsEmpty() const override;
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void EnsureBufferReady(
    bool isToComplete, GOSchedulerThread *pThread = nullptr) override;

  void DiscardContent() override;
  void Add(GOSoundSampler *sampler);
  void WaitAndDiscardContent();
};

#endif
