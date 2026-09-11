/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundWindchestGroupTask.h"

#include <cassert>

#include "scheduler/GOSchedulerThread.h"
#include "sound/playing/GOSoundSamplerPlayer.h"
#include "threading/GOMutexLocker.h"

#include "GOSoundWindchestTask.h"

GOSoundWindchestGroupTask::GOSoundWindchestGroupTask(
  const GORoundCounter &roundCounter,
  GOSoundSamplerPlayer &samplerPlayer,
  GOSoundWindchestTask &windchestTask,
  unsigned nFramesPerBuffer)
  : GOSoundBufferTaskBase(PRIORITY_WINDCHESTMIX, true, 2, nFramesPerBuffer),
    r_RoundCounter(roundCounter),
    r_SamplerPlayer(samplerPlayer),
    r_WindchestTask(windchestTask),
    m_Condition(m_mutex),
    m_ActiveCount(0) {}

void GOSoundWindchestGroupTask::DiscardContent() {
  m_Active.Clear();
  m_Release.Clear();
}

void GOSoundWindchestGroupTask::Add(GOSoundSampler *sampler) {
  if (sampler->is_release)
    m_Release.Put(sampler);
  else
    m_Active.Put(sampler);
}

void GOSoundWindchestGroupTask::ProcessList(
  GOSoundSamplerList &list, bool isToDropOld, GOSoundBufferMutable &outBuffer) {
  GOSoundSampler *sampler;

  while ((sampler = list.Get())) {
    if (
      isToDropOld && m_IsToComplete.load()
      && sampler->time + 2000 < r_SamplerPlayer.GetTime()) {
      if (sampler->drop_counter++ > 3) {
        r_SamplerPlayer.ReturnSampler(sampler);
        continue;
      }
    }
    sampler->drop_counter = 0;

    GOSoundWindchestTask *const windchest = sampler->p_WindchestTask;

    if (
      windchest
      && r_SamplerPlayer.ProcessSampler(
        *sampler, windchest->GetAmplitude(), outBuffer))
      Add(sampler);
  }
}

unsigned GOSoundWindchestGroupTask::GetCost() const {
  return m_Active.GetCount() + m_Release.GetCount();
}

void GOSoundWindchestGroupTask::Run(GOSchedulerThread *pThread) {
  if (m_RunState.load() < RUN_STATE_DONE) {
    bool isParticipating = false;

    {
      GOMutexLocker locker(
        m_mutex,
        false,
        "GOSoundWindchestGroupTask::Run.beforeProcess",
        pThread);

      if (locker.IsLocked()) {
        if (m_RunState.load() == RUN_STATE_NOT_STARTED) {
          // the first thread entered Run() claims the round
          m_Active.Move();
          m_Release.Move();
          m_RunState.store(RUN_STATE_IN_PROGRESS);
          isParticipating = true;
        } else if (m_Active.Peek() || m_Release.Peek())
          isParticipating = true;

        if (isParticipating)
          m_ActiveCount.fetch_add(1);
      }
    }

    if (isParticipating) {
      // several threads may process the same list in parallel helping each
      // other; at first, they fill their's own buffer instances
      GO_DECLARE_LOCAL_SOUND_BUFFER(localBuffer, 2, GetNFrames())

      localBuffer.FillWithSilence();
      ProcessList(m_Active, false, localBuffer);
      ProcessList(m_Release, true, localBuffer);

      GOMutexLocker locker(
        m_mutex, false, "GOSoundWindchestGroupTask::Run.afterProcess", pThread);

      if (locker.IsLocked()) {
        if (m_RunState.load() == RUN_STATE_IN_PROGRESS) {
          // The first thread is finished. Assign the result to the common
          // buffer
          CopyFrom(localBuffer);
          m_RunState.store(RUN_STATE_PARTLY_DONE);
        } else
          // not the first thread. Add the result to the common buffer
          AddFrom(localBuffer);
      }
      if (m_ActiveCount.fetch_sub(1) <= 1) {
        // the last thread
        m_RunState.store(RUN_STATE_DONE);
        m_Condition.Broadcast();
      }
    }
  }
}

void GOSoundWindchestGroupTask::EnsureBufferReady(
  bool isToComplete, GOSchedulerThread *pThread) {
  if (isToComplete)
    m_IsToComplete.store(true);

  // Captured before Run(): a NewRound() landing between Run() and the
  // re-check below would otherwise be invisible - see GORoundCounter and
  // the identical fix on GOSoundGroupTask::EnsureBufferReady() (the class
  // this cooperative round protocol was ported from).
  const uint64_t roundNumber = r_RoundCounter.GetRoundNumber();

  Run(pThread);
  if (m_RunState.load() < RUN_STATE_DONE) {
    GOMutexLocker locker(
      m_mutex, false, "GOSoundWindchestGroupTask::EnsureBufferReady", pThread);

    while (locker.IsLocked()
           && m_RunState.load() < RUN_STATE_DONE
           // Leave once the round changes underneath us: RUN_STATE_NOT_STARTED
           // then means "my round is over and this is the next one", not "my
           // round has not started yet" - and nothing will ever finish a
           // round this call did not ask to join.
           && r_RoundCounter.GetRoundNumber() == roundNumber
           && (pThread == nullptr || !pThread->ShouldStop()))
      m_Condition.WaitWithTimeout("GOSoundWindchestGroupTask::EnsureBufferReady");
  }
}

void GOSoundWindchestGroupTask::CompleteRound() {
  // precisely the round deadline: pass it down, finish the round, and do not
  // return until it is finished
  EnsureBufferReady(true);
}

// Called under m_mutex from GOSoundTaskBase::NewRound(), after m_RunState is
// already reset to RUN_STATE_NOT_STARTED there, so the m_RunState assertion
// below is trivially true and only documents the precondition; m_ActiveCount
// is the assertion that actually matters. This task mixes in ProcessList()
// outside m_mutex, so the mutex alone cannot keep a worker out of a round
// being reset: the protocol must already have brought the task to rest by
// the time NewRound() is called - either it never ran this period, or
// CompleteRound() ran it to completion.
void GOSoundWindchestGroupTask::DoNewRound() {
  assert(m_ActiveCount.load() == 0);
  assert(
    m_RunState.load() == RUN_STATE_NOT_STARTED
    || m_RunState.load() == RUN_STATE_DONE);
  // not redundant with the assertion above: assert() is compiled out under
  // NDEBUG, and resetting the counter is what this hook exists to do
  m_ActiveCount.store(0);
  // Wakes anyone parked in EnsureBufferReady() on the round being
  // superseded so that it leaves on the round-number check immediately,
  // instead of after a full WaitWithTimeout() period.
  m_Condition.Broadcast();
}

// Read without m_mutex, like every other IsEmpty(): called only while the
// task is quiescent (deregistered from the scheduler), never concurrently
// with Run()
bool GOSoundWindchestGroupTask::IsEmpty() const {
  return m_Active.IsEmpty() && m_Release.IsEmpty();
}

void GOSoundWindchestGroupTask::WaitAndDiscardContent() {
  GOMutexLocker locker(m_mutex, false, "WaitAndDiscardContent");

  // wait for no threads are inside Run()
  while (m_RunState.load() > RUN_STATE_NOT_STARTED
         && m_RunState.load() < RUN_STATE_DONE)
    m_Condition.WaitWithTimeout("WaitAndDiscardContent");

  // Now it is safe to clear because m_mutex is locked and no other threads
  // can enter in Run()
  DiscardContent();
}
