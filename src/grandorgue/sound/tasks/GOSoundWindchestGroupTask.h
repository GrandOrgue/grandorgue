/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDWINDCHESTGROUPTASK_H
#define GOSOUNDWINDCHESTGROUPTASK_H

#include <atomic>
#include <memory>

#include "scheduler/GORoundCounter.h"
#include "sound/playing/GOSoundSamplerList.h"
#include "threading/GOCondition.h"

#include "GOSoundBufferTaskBase.h"

class GOSchedulerThread;
class GOSoundBufferMutable;
class GOSoundProcessingChainState;
class GOSoundSamplerPlayer;
class GOSoundWindchestTask;

class GOSoundWindchestGroupTask : public GOSoundBufferTaskBase {
private:
  const GORoundCounter &r_RoundCounter;
  GOSoundSamplerPlayer &r_SamplerPlayer;
  /** The windchest this cell belongs to; non-owning, must outlive this task.
   * Used to force its amplitude/chain-parameter computation to completion
   * before mixing (see GOSoundWindchestTask::GetAmplitude()) and to build
   * mp_ChainState from this windchest's chain. */
  GOSoundWindchestTask &r_WindchestTask;
  GOSoundSamplerList m_Active;
  GOSoundSamplerList m_Release;
  GOCondition m_Condition;
  /** This cell's own DSP memory for r_WindchestTask's chain; built from it
   * at construction. Never null (an empty chain still yields a valid,
   * empty-loop state). Run once per round by OnMixed(). */
  std::unique_ptr<GOSoundProcessingChainState> mp_ChainState;

  // the number of threads that are processing the samples
  std::atomic_uint m_ActiveCount;

  void ProcessList(
    GOSoundSamplerList &list,
    bool isToDropOld,
    GOSoundBufferMutable &outBuffer);

  /**
   * Runs r_WindchestTask's chain, in place, on this cell's fully-merged
   * buffer - exactly once per round, called by the last thread into Run()
   * before the round is published as RUN_STATE_DONE. Forces
   * r_WindchestTask's own round to completion first (GetAmplitude()): a
   * round with zero samplers never otherwise triggers it, but a stateful
   * processor (e.g. a reverb tail) still needs current parameters and must
   * still run.
   *
   * Called from Run() while still holding this cell's own m_mutex - the
   * locker at that call site always blocks (no pThread passed), so the lock
   * is unconditionally held - and in turn takes r_WindchestTask's own mutex
   * via GetAmplitude(). There is no cycle today - GOSoundWindchestTask::
   * DoRun() only touches tremulant tasks and the chain's own parameter
   * mappers, none of which lock back into a GOSoundWindchestGroupTask - but
   * any future model-aware processor/mapper that reaches back into a
   * windchest-group task from inside the chain would deadlock against this
   * lock order and must not do so.
   *
   * Known pre-existing hazard (inherited from the general task Run()
   * protocol this class reuses, not introduced here): a thread that enters
   * Run() after the last participant's fetch_sub<=1 but before m_RunState is
   * stored as RUN_STATE_DONE can, in principle, be treated as a fresh
   * participant and cause a second ProcessList()/OnMixed() pass within the
   * same round. Previously this only risked double-mixing a sampler; now it
   * can also advance a stateful processor's state (e.g. a reverb tail)
   * twice in one period. Not fixed here - documenting the increased cost per
   * review, not resolving the underlying protocol race.
   */
  void OnMixed();

  /**
   * Stores RUN_STATE_DONE and broadcasts m_Condition - called by the last
   * thread into Run(), after OnMixed(), with m_mutex already held by the
   * caller (the locker at that single call site always blocks, so the lock
   * is unconditionally held). Factored out only for readability, not
   * because it is reused elsewhere.
   */
  void PublishDone();

public:
  GOSoundWindchestGroupTask(
    const GORoundCounter &roundCounter,
    GOSoundSamplerPlayer &samplerPlayer,
    GOSoundWindchestTask &windchestTask,
    unsigned nFramesPerBuffer);
  ~GOSoundWindchestGroupTask();

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
   * once. It only really blocks for a (windchest, audio group) cell whose
   * audio group has zero scale factors in every output, which is the case
   * nothing else waits for.
   */
  void CompleteRound() override;

private:
  /**
   * Resets the active-worker count for the next round. Asserts first that the
   * round protocol really brought this task to rest, which the mutex
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
