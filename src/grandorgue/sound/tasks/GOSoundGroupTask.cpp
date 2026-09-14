/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundGroupTask.h"

#include "scheduler/GOSchedulerThread.h"
#include "sound/playing/GOSoundSamplerPlayer.h"
#include "threading/GOMutexLocker.h"

#include "GOSoundWindchestTask.h"

GOSoundGroupTask::GOSoundGroupTask(
  const GORoundCounter &roundCounter,
  GOSoundSamplerPlayer &samplerPlayer,
  unsigned nFramesPerBuffer)
  : GOSoundBufferTaskBase(PRIORITY_AUDIOGROUP, true, 2, nFramesPerBuffer),
    r_RoundCounter(roundCounter),
    r_SamplerPlayer(samplerPlayer),
    m_Condition(m_mutex),
    m_ActiveCount(0) {}

void GOSoundGroupTask::DoNewRound() {
  m_ActiveCount.store(0);
  // Called from NewRound() while it holds m_mutex: wake anyone parked in
  // EnsureBufferReady() on the round being superseded so that it leaves on
  // the round-number check immediately, instead of after a full
  // WaitWithTimeout() period.
  m_Condition.Broadcast();
}

void GOSoundGroupTask::DiscardContent() {
  m_Active.Clear();
  m_Release.Clear();
}

void GOSoundGroupTask::Add(GOSoundSampler *sampler) {
  if (sampler->is_release)
    m_Release.Put(sampler);
  else
    m_Active.Put(sampler);
}

void GOSoundGroupTask::ProcessList(
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

unsigned GOSoundGroupTask::GetCost() const {
  return m_Active.GetCount() + m_Release.GetCount();
}

// Read without m_mutex, like every other IsEmpty(): called only while the
// task is quiescent (deregistered from the scheduler), never concurrently
// with Run()
bool GOSoundGroupTask::IsEmpty() const {
  return m_Active.IsEmpty() && m_Release.IsEmpty();
}

void GOSoundGroupTask::Run(GOSchedulerThread *pThread) {
  if (m_RunState.load() < RUN_STATE_DONE) {
    bool isParticipating = false;

    {
      GOMutexLocker locker(
        m_mutex, false, "GOSoundGroupTask::Run.beforeProcess", pThread);

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
        m_mutex, false, "GOSoundGroupTask::Run.afterProcess", pThread);

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

void GOSoundGroupTask::EnsureBufferReady(
  bool isToComplete, GOSchedulerThread *pThread) {
  if (isToComplete)
    m_IsToComplete.store(true);

  // Captured before Run(): a NewRound() landing between Run() and the
  // re-check below would otherwise be invisible.
  const uint64_t roundNumber = r_RoundCounter.GetRoundNumber();

  Run(pThread);
  if (m_RunState.load() < RUN_STATE_DONE) {
    GOMutexLocker locker(
      m_mutex, false, "GOSoundGroupTask::EnsureBufferReady", pThread);

    while (locker.IsLocked()
           && m_RunState.load() < RUN_STATE_DONE
           // Leave once the round changes underneath us: RUN_STATE_NOT_STARTED
           // then means "my round is over and this is the next one", not "my
           // round has not started yet" - and nothing will ever finish a
           // round this call did not ask to join.
           && r_RoundCounter.GetRoundNumber() == roundNumber
           && (pThread == nullptr || !pThread->ShouldStop()))
      m_Condition.WaitWithTimeout("GOSoundGroupTask::EnsureBufferReady");
  }
}

void GOSoundGroupTask::WaitAndDiscardContent() {
  GOMutexLocker locker(m_mutex, false, "WaitAndDiscardContent");

  // wait for no threads are inside Run()
  while (m_RunState.load() > RUN_STATE_NOT_STARTED
         && m_RunState.load() < RUN_STATE_DONE)
    m_Condition.WaitWithTimeout("WaitAndDiscardContent");

  // Now it is safe to clear because m_mutex is locked and no other threads
  // can enter in Run()
  DiscardContent();
}
