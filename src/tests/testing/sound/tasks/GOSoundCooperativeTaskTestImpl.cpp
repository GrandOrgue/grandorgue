/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundCooperativeTaskTestImpl.h"

#include "threading/GOMutexLocker.h"

long GOSoundCooperativeTaskTestImpl::DoOwnShare() {
  const long nItemsPerThread = m_WorkItemsPerThread.load();
  long nProcessedItems = 0;

  if (nItemsPerThread > 0) {
    // Per-thread quota. The accumulator is volatile so that the loop cannot
    // be folded away: each item must stay real work, but work that touches
    // only this thread's stack.
    volatile long sink = 0;

    for (long itemI = 0; itemI < nItemsPerThread; itemI++)
      sink = sink + 1;
    nProcessedItems = sink;
  } else
    // Shared pool, drained one item at a time, as ProcessList() drains a
    // sampler list.
    while (m_RemainingWorkItems.fetch_sub(1) > 0)
      nProcessedItems++;

  return nProcessedItems;
}

// Structurally mirrors GOSoundGroupTask::Run(), including the fact that the
// active-thread bookkeeping sits outside the IsLocked() check.
void GOSoundCooperativeTaskTestImpl::Run(GOSchedulerThread *pThread) {
  if (m_RunState.load() < RUN_STATE_DONE) {
    bool isParticipating = false;

    {
      GOMutexLocker locker(
        m_mutex, false, "GOSoundCooperativeTaskTestImpl::Run.before", pThread);

      if (locker.IsLocked()) {
        if (m_RunState.load() == RUN_STATE_NOT_STARTED) {
          // the first thread entered Run() claims the round
          m_RunState.store(RUN_STATE_IN_PROGRESS);
          nFirstTransitions.fetch_add(1);
          isParticipating = true;
        } else if (
          m_WorkItemsPerThread.load() > 0 || m_RemainingWorkItems.load() > 0)
          // as in ProcessList(): join only if there is something to help with
          isParticipating = true;

        if (isParticipating)
          m_ActiveCount.fetch_add(1);
      }
    }

    if (isParticipating) {
      const long localValue = DoOwnShare();

      nTotalProcessedItems.fetch_add(localValue);

      GOMutexLocker locker(
        m_mutex, false, "GOSoundCooperativeTaskTestImpl::Run.after", pThread);

      if (locker.IsLocked()) {
        if (m_RunState.load() == RUN_STATE_IN_PROGRESS) {
          // the first thread finished: its share becomes the shared result
          nSharedValue.store(localValue);
          m_RunState.store(RUN_STATE_PARTLY_DONE);
          nCopyTransitions.fetch_add(1);
        } else
          // not the first thread: merge into the shared result
          nSharedValue.fetch_add(localValue);
      }
      if (m_ActiveCount.fetch_sub(1) <= 1) {
        // the last thread
        m_RunState.store(RUN_STATE_DONE);
        nLastThreadFinishes.fetch_add(1);
        m_Condition.Broadcast();
      }
    }
  }
}

void GOSoundCooperativeTaskTestImpl::WaitUntilDone() {
  GOMutexLocker locker(m_mutex);

  while (m_RunState.load() != RUN_STATE_DONE)
    m_Condition.WaitOrStop();
}
