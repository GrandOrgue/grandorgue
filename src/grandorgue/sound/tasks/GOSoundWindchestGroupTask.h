/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTGROUPTASK_H
#define GOSOUNDWINDCHESTGROUPTASK_H

#include <atomic>

#include "scheduler/GORoundCounter.h"
#include "sound/playing/GOSoundSamplerList.h"
#include "threading/GOCondition.h"

#include "GOSoundBufferTaskBase.h"

class GOSoundBufferMutable;
class GOSoundSamplerPlayer;
class GOSoundWindchestTask;

class GOSoundWindchestGroupTask : public GOSoundBufferTaskBase {
private:
  const GORoundCounter &r_RoundCounter;
  GOSoundSamplerPlayer &r_SamplerPlayer;
  /** The windchest this cell belongs to; non-owning, must outlive this task.
   * Used to force its amplitude/chain-parameter computation to completion
   * before mixing (see GOSoundWindchestTask::GetAmplitude()) and, once this
   * class gains its own processing chain state, to build that state from
   * this windchest's chain. */
  GOSoundWindchestTask &r_WindchestTask;
  GOSoundSamplerList m_Active;
  GOSoundSamplerList m_Release;
  GOCondition m_Condition;

  // the number of threads that are processing the samples
  std::atomic_uint m_ActiveCount;

  void ProcessList(
    GOSoundSamplerList &list,
    bool isToDropOld,
    GOSoundBufferMutable &outBuffer);

public:
  GOSoundWindchestGroupTask(
    const GORoundCounter &roundCounter,
    GOSoundSamplerPlayer &samplerPlayer,
    GOSoundWindchestTask &windchestTask,
    unsigned nFramesPerBuffer);

  unsigned GetCost() const override;
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void EnsureBufferReady(
    bool isToComplete, GOSchedulerThread *pThread = nullptr) override;

  /**
   * Unlike the base implementation, does not return until the round has
   * actually reached RUN_STATE_DONE.
   *
   * This task mixes in ProcessList() outside m_mutex, so the mutex NewRound()
   * takes does not exclude a worker that is still mixing. Without waiting
   * here, NewRound() could reset the round under such a worker, which would
   * then merge the previous period into the new period's buffer, mark the
   * fresh round done before anything was mixed into it, and underflow
   * m_ActiveCount. Waiting makes "the round is over" true by the time
   * GOScheduler::CompleteRoundList() returns, which is what NewRound()
   * relies on.
   *
   * Normally free: GetAudioOutput() has already driven this task to
   * RUN_STATE_DONE before NextPeriod() is entered, so the wait returns at
   * once. It only really blocks for an audio group whose scale factors are
   * zero in every output, which is the case nothing else waits for.
   */
  void CompleteRound() override;

private:
  /**
   * Resets the active-worker count for the next round. Asserts first that the
   * round protocol really brought this task to rest (via m_ActiveCount; the
   * m_RunState half of the assertion is trivially true here, since
   * NewRound() has already reset it by the time this runs), which the mutex
   * NewRound() holds cannot guarantee on its own - see the .cpp. Also
   * broadcasts, so a thread parked in EnsureBufferReady() on the round being
   * superseded (see r_RoundCounter above) leaves immediately instead of
   * after a full WaitWithTimeout() period.
   */
  void DoNewRound() override;

public:
  void Add(GOSoundSampler *sampler);
  void DiscardContent() override;
  void WaitAndDiscardContent();

  /** @return true if this task has no active or releasing samplers -
   * overrides the base class's round-state-only check, which cannot see
   * sampler-list content. Read without m_mutex, like every other IsEmpty(). */
  bool IsEmpty() const override;
};

#endif
