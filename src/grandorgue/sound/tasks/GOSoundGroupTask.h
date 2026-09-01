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
#include "threading/GOMutex.h"

#include "GOSoundBufferTaskBase.h"

class GOSchedulerThread;
class GOSoundBufferMutable;
class GOSoundSamplerPlayer;

class GOSoundGroupTask : public GOSoundBufferTaskBase {
private:
  GOSoundSamplerPlayer &r_SamplerPlayer;
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
  std::atomic_bool m_IsToComplete;

  void ProcessList(
    GOSoundSamplerList &list,
    bool isToDropOld,
    GOSoundBufferMutable &outBuffer);

public:
  GOSoundGroupTask(
    GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer);

  unsigned GetPriority() const override { return PRIORITY_AUDIOGROUP; }
  unsigned GetCost() const override;
  bool IsRepeatable() const override { return true; }
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void CompleteRound() override;
  void EnsureBufferReady(
    bool isToComplete, GOSchedulerThread *pThread = nullptr) override;

  void NewRound() override;
  void DiscardContent() override;
  void Add(GOSoundSampler *sampler);
  void WaitAndDiscardContent();
};

#endif
