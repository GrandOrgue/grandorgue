/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDCOOPERATIVETASKTESTIMPL_H
#define GOSOUNDCOOPERATIVETASKTESTIMPL_H

#include <atomic>
#include <vector>

#include "scheduler/GOSchedulerThread.h"
#include "sound/tasks/GOSoundTaskBase.h"
#include "threading/GOCondition.h"

/**
 * Test double reproducing the cooperative Run() protocol of
 * GOSoundGroupTask::Run() (several worker threads jointly processing one
 * round, merging into one shared result), without touching the sound
 * engine or real sampler lists. Shared between GOTestSoundTaskBase.cpp
 * (correctness) and GOTestPerfSoundTaskBase.cpp (throughput), both of which
 * include this header, so its definition is identical in every translation
 * unit that uses it.
 */
class GOSoundCooperativeTaskTestImpl : public GOSoundTaskBase {
private:
  /** Wakes a thread waiting in WaitUntilDone() once the round is done */
  GOCondition m_Condition{m_mutex};
  /** Number of threads currently doing their own share of the round */
  std::atomic<int> m_ActiveCount{0};
  /** Stands in for the sampler lists a real GOSoundGroupTask drains */
  std::atomic<long> m_RemainingWorkItems{0};
  /** If > 0, each participating thread does this many items of purely
   * thread-local work instead of draining m_RemainingWorkItems */
  std::atomic<long> m_WorkItemsPerThread{0};
  /** Stands in for the buffer GOSoundGroupTask merges into under m_mutex */
  std::vector<float> m_MergeBuffer;

  /** Merges this thread's share into the shared result, mirroring the
   * CopyFrom()/AddFrom() pass GOSoundGroupTask does while holding m_mutex
   * @param localValue this thread's own result
   * @param isFirst whether this thread is the first to finish its share */
  void MergeOwnShare(long localValue, bool isFirst);

  /** Does this thread's share of the round's work
   * @return the number of items this thread processed */
  long DoOwnShare();

public:
  /** Stands in for the shared output buffer the real task mixes into */
  std::atomic<long> nSharedValue{0};
  /** Number of RUN_STATE_NOT_STARTED -> RUN_STATE_IN_PROGRESS transitions */
  std::atomic<int> nFirstTransitions{0};
  /** Number of RUN_STATE_IN_PROGRESS -> RUN_STATE_PARTLY_DONE transitions */
  std::atomic<int> nCopyTransitions{0};
  /** Number of transitions to RUN_STATE_DONE */
  std::atomic<int> nLastThreadFinishes{0};
  /** Sum of DoOwnShare()'s return value across every participating thread on
   * every round so far. Not every thread that calls Run() participates: one
   * arriving after the round is already RUN_STATE_DONE does no work, so a
   * caller measuring throughput must count this rather than assume every
   * thread processed a full share */
  std::atomic<long> nTotalProcessedItems{0};

  GOSoundCooperativeTaskTestImpl()
    : GOSoundTaskBase(PRIORITY_AUDIOGROUP, true) {}

  /** Sets the amount of work the next round has to process, as one shared
   * pool that every participating thread drains item by item. Every item is
   * a contended atomic operation on one cache line, which is what the
   * correctness tests need in order to prove that no item is lost or
   * counted twice */
  void SetWorkItems(long n) { m_RemainingWorkItems.store(n); }

  /** Switches the round to a per-thread quota: each participating thread
   * does n items of thread-local work and the shared state is touched once
   * per round rather than once per item. Used by the perf tests, where
   * draining one shared counter would measure cache-line ping-pong on that
   * counter instead of the cost of the round protocol itself */
  void SetWorkItemsPerThread(long n) { m_WorkItemsPerThread.store(n); }

  /** Gives the merge section a realistic cost: a linear pass over nItems
   * floats, as GOSoundGroupTask's CopyFrom()/AddFrom() does over its output
   * buffer while holding m_mutex. Without it the merge is a single atomic
   * operation, and a measurement of how long the entry section waits behind
   * someone else's merge is meaningless. Not thread-safe: call before the
   * round starts */
  void SetMergeItems(unsigned nItems) { m_MergeBuffer.assign(nItems, 0.0f); }

  /** Joins or claims the current round, does a share of the work, then
   * merges the result; the last thread to finish marks the round done */
  void Run(GOSchedulerThread *pThread = nullptr) override;

  /** Resets the active-worker count for the next round */
  void DoNewRound() override { m_ActiveCount.store(0); }

  /** Blocks the calling thread until the round becomes done, mirroring
   * GOSoundGroupTask::WaitAndDiscardContent() */
  void WaitUntilDone();
};

#endif /* GOSOUNDCOOPERATIVETASKTESTIMPL_H */
