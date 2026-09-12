/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundGroupTask.h"

#include <cassert>

#include "scheduler/GOSchedulerThread.h"
#include "sound/playing/GOSoundSamplerPlayer.h"
#include "threading/GOMutexLocker.h"

#include "GOSoundWindchestTask.h"

GOSoundGroupTask::GOSoundGroupTask(
  GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer)
  : GOSoundBufferTaskBase(PRIORITY_AUDIOGROUP, true, 2, nFramesPerBuffer),
    r_SamplerPlayer(samplerPlayer),
    m_Condition(m_mutex),
    m_ActiveCount(0) {}

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

      // Blocking on purpose, unlike every other GOMutexLocker in this class:
      // this thread already incremented m_ActiveCount above and computed its
      // share into localBuffer, so merging it and (if this turns out to be
      // the last share) publishing RUN_STATE_DONE must not be skippable via
      // pThread->ShouldStop() - a stopping worker bailing here would either
      // drop its own share silently (the merge is skipped) or, worse,
      // publish RUN_STATE_DONE while another thread is still concurrently
      // merging its own share under the lock, since the two would no longer
      // be serialised against each other at all.
      GOMutexLocker locker(
        m_mutex, false, "GOSoundGroupTask::Run.afterProcess");

      if (m_RunState.load() == RUN_STATE_IN_PROGRESS) {
        // The first thread is finished. Assign the result to the common
        // buffer
        CopyFrom(localBuffer);
        m_RunState.store(RUN_STATE_PARTLY_DONE);
      } else
        // not the first thread. Add the result to the common buffer
        AddFrom(localBuffer);
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
  Run(pThread);
  if (m_RunState.load() < RUN_STATE_DONE) {
    GOMutexLocker locker(
      m_mutex, false, "GOSoundGroupTask::EnsureBufferReady", pThread);

    while (locker.IsLocked() && m_RunState.load() < RUN_STATE_DONE
           && (pThread == nullptr || !pThread->ShouldStop()))
      m_Condition.WaitOrStop("GOSoundGroupTask::EnsureBufferReady", pThread);
  }
}

void GOSoundGroupTask::CompleteRound() {
  // precisely the round deadline: pass it down, finish the round, and do not
  // return until it is finished
  EnsureBufferReady(true);
}

// Called under m_mutex from GOSoundTaskBase::NewRound(), before the round
// state is reset, so the assertions still see the round that is ending. This
// task mixes in ProcessList() outside m_mutex, so the mutex alone cannot keep
// a worker out of a round being reset: the protocol must already have brought
// the task to rest. Either it never ran this period (RUN_STATE_NOT_STARTED),
// or CompleteRound() ran it to completion (RUN_STATE_DONE).
void GOSoundGroupTask::DoNewRound() {
  assert(m_ActiveCount.load() == 0);
  assert(
    m_RunState.load() == RUN_STATE_NOT_STARTED
    || m_RunState.load() == RUN_STATE_DONE);
  // not redundant with the assertion above: assert() is compiled out under
  // NDEBUG, and resetting the counter is what this hook exists to do
  m_ActiveCount.store(0);
}

void GOSoundGroupTask::WaitAndDiscardContent() {
  GOMutexLocker locker(m_mutex, false, "WaitAndDiscardContent");

  // wait for no threads are inside Run()
  while (m_RunState.load() > RUN_STATE_NOT_STARTED
         && m_RunState.load() < RUN_STATE_DONE)
    m_Condition.WaitOrStop("WaitAndDiscardContent", NULL);

  // Now it is safe to clear because m_mutex is locked and no other threads
  // can enter in Run()
  DiscardContent();
}
