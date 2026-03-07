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
  GOSoundSamplerPlayer &samplerPlayer, unsigned nFramesPerBuffer)
  : GOSoundBufferTaskBase(2, nFramesPerBuffer),
    r_SamplerPlayer(samplerPlayer),
    m_Condition(m_Mutex),
    m_ActiveCount(0),
    m_Done(0),
    m_IsToComplete(false) {}

void GOSoundGroupTask::NewRound() {
  GOMutexLocker locker(m_Mutex);
  m_Done.store(0);
  m_ActiveCount.store(0);
  m_IsToComplete.store(false);
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

void GOSoundGroupTask::Run(GOSchedulerThread *pThread) {
  if (m_Done.load() == 3) // has already processed in this period
    return;
  {
    GOMutexLocker locker(
      m_Mutex, false, "GOSoundGroupTask::Run.beforeProcess", pThread);

    if (!locker.IsLocked())
      return;

    if (m_Done.load() == 0) // the first thread entered to Run()
    {
      m_Active.Move();
      m_Release.Move();
      m_Done.store(1); // there are some thteads in Run()
    } else {
      if (!m_Active.Peek() && !m_Release.Peek())
        return;
    }
    m_ActiveCount.fetch_add(1);
  }

  // several threads may process the same list in parallel helping each other
  // at first, they fill their's own buffer instances
  GO_DECLARE_LOCAL_SOUND_BUFFER(localBuffer, 2, GetNFrames())

  localBuffer.FillWithSilence();
  ProcessList(m_Active, false, localBuffer);
  ProcessList(m_Release, true, localBuffer);

  {
    GOMutexLocker locker(
      m_Mutex, false, "GOSoundGroupTask::Run.afterProcess", pThread);

    if (locker.IsLocked()) {
      if (m_Done.load() == 1) {
        // The first thread is finished. Assign the result to the common buffer
        CopyFrom(localBuffer);
        m_Done.store(2); // some thread has already finished
      } else {
        // not the first thread. Add the result to the common buffer
        AddFrom(localBuffer);
      }
    }
    if (m_ActiveCount.fetch_sub(1) <= 1) {
      // the last thread
      m_Done.store(3); // all threads have finished processing this period
      m_Condition.Broadcast();
    }
  }
}

void GOSoundGroupTask::CompleteRound() {
  m_IsToComplete.store(true);
  Run();
}

void GOSoundGroupTask::EnsureBufferReady(
  bool isToComplete, GOSchedulerThread *pThread) {
  if (isToComplete)
    m_IsToComplete.store(true);
  Run(pThread);
  if (m_Done.load() == 3)
    return;

  {
    GOMutexLocker locker(
      m_Mutex, false, "GOSoundGroupTask::EnsureBufferReady", pThread);

    while (locker.IsLocked() && m_Done.load() != 3
           && (pThread == nullptr || !pThread->ShouldStop()))
      m_Condition.WaitOrStop("GOSoundGroupTask::EnsureBufferReady", pThread);
  }
}

void GOSoundGroupTask::WaitAndDiscardContent() {
  GOMutexLocker locker(m_Mutex, false, "WaitAndDiscardContent");

  // wait for no threads are inside Run()
  while (m_Done.load() > 0 && m_Done.load() < 3)
    m_Condition.WaitOrStop("WaitAndDiscardContent", NULL);

  // Now it is safe to clear because m_Mutex is locked and no other threads can
  // enter in Run()
  DiscardContent();
}
