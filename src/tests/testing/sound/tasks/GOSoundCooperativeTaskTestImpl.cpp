/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundCooperativeTaskTestImpl.h"

#include <cassert>
#include <chrono>
#include <thread>

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

  if (shareSleepMicroseconds.load() > 0)
    // linger outside any lock, as a worker inside ProcessList() does
    std::this_thread::sleep_for(
      std::chrono::microseconds(shareSleepMicroseconds.load()));

  return nProcessedItems;
}

// Mirrors what GOSoundGroupTask does under m_mutex once a thread has finished
// its share: the first one to arrive copies its result in, everyone after it
// adds to what is already there. The linear pass over m_MergeBuffer is there
// so that the section costs what a buffer merge costs; with an empty buffer
// it degenerates to the bookkeeping alone.
void GOSoundCooperativeTaskTestImpl::MergeOwnShare(
  long localValue, bool isFirst) {
  const float share = static_cast<float>(localValue);

  if (isFirst) {
    for (float &item : m_MergeBuffer)
      item = share;
    nSharedValue.store(localValue);
  } else {
    for (float &item : m_MergeBuffer)
      item += share;
    nSharedValue.fetch_add(localValue);
  }
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
          m_RunState.load() < RUN_STATE_DONE
          && (m_WorkItemsPerThread.load() > 0 || m_RemainingWorkItems.load() > 0))
          // As in ProcessList(): join only if there is something to help with.
          // The state has to be re-checked here even though Run() already
          // checked it above: that check is outside m_mutex, so a thread may
          // have passed it while the round was still running and only reach
          // this point after the last worker finished the round and published
          // RUN_STATE_DONE from inside the mutex. Joining then would raise
          // m_ActiveCount on a round already declared over, which is what
          // DoNewRound() asserts against. GOSoundGroupTask is safe from this
          // for a reason that does not carry over to a work quota: its
          // condition is m_Active.Peek() || m_Release.Peek(), and both lists
          // are drained by the time the round is done, so a late thread finds
          // nothing to help with and leaves.
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
        const bool isFirst = m_RunState.load() == RUN_STATE_IN_PROGRESS;

        MergeOwnShare(localValue, isFirst);
        if (isFirst) {
          // the first thread finished: its share became the shared result
          m_RunState.store(RUN_STATE_PARTLY_DONE);
          nCopyTransitions.fetch_add(1);
        }
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

void GOSoundCooperativeTaskTestImpl::CompleteRound() {
  GOSoundTaskBase::CompleteRound();
  WaitUntilDone();
}

// Mirrors GOSoundGroupTask::DoNewRound(), so that this double breaks wherever
// the production task would: the round protocol, not m_mutex, is what has to
// guarantee no worker is still inside its share by the time NewRound() runs.
void GOSoundCooperativeTaskTestImpl::DoNewRound() {
  assert(m_ActiveCount.load() == 0);
  assert(
    m_RunState.load() == RUN_STATE_NOT_STARTED
    || m_RunState.load() == RUN_STATE_DONE);
  m_ActiveCount.store(0);
}

void GOSoundCooperativeTaskTestImpl::WaitUntilDone() {
  GOMutexLocker locker(m_mutex);

  while (m_RunState.load() != RUN_STATE_DONE)
    m_Condition.WaitOrStop();
}
