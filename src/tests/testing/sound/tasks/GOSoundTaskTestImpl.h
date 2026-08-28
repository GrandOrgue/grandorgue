/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTASKTESTIMPL_H
#define GOSOUNDTASKTESTIMPL_H

#include <atomic>

#include "scheduler/GOSchedulerThread.h"
#include "sound/tasks/GOSoundTaskBase.h"

/**
 * Test double for the default, non-cooperative GOSoundTaskBase protocol
 * (Run()/DoRun()/DoNewRound()). Shared between GOTestSoundTaskBase.cpp
 * (correctness) and GOTestPerfSoundTaskBase.cpp (throughput/latency), both
 * of which include this header, so its definition is identical in every
 * translation unit that uses it.
 *
 * The bookkeeping counters below are plain atomics, cheap enough (a few ns)
 * to stay negligible next to what the perf tests actually measure (lock
 * acquisition, thread scheduling, microseconds-scale simulated work).
 */
class GOSoundTaskTestImpl : public GOSoundTaskBase {
public:
  /** Number of DoRun() calls observed so far */
  std::atomic<int> nDoRunCalls{0};
  /** Number of DoRun() calls currently in flight, for detecting overlap */
  std::atomic<int> nConcurrentDoRunCalls{0};
  /** Peak value ever reached by nConcurrentDoRunCalls */
  std::atomic<int> nMaxConcurrentDoRunCalls{0};
  /** Number of DoNewRound() calls observed so far */
  std::atomic<int> nDoNewRoundCalls{0};
  /** Value DoRun() returns; set by the test before calling Run() */
  std::atomic<bool> doRunResult{true};
  /**
   * If > 0, DoRun() sleeps this long. Use only to widen a race window: the
   * sleeping thread is descheduled while still holding m_mutex, which is
   * fine for a correctness test but would make a perf scenario measure
   * wake-up granularity rather than the round protocol.
   */
  std::atomic<int> doRunSleepMicroseconds{0};
  /**
   * If > 0, DoRun() busy-waits this long, standing in for real per-round
   * work. Unlike doRunSleepMicroseconds the lock holder stays runnable, so
   * what gets measured is the protocol rather than the scheduler. At most
   * one thread is ever inside DoRun(), so this never spins on more than one
   * core.
   */
  std::atomic<int> doRunWorkMicroseconds{0};
  /** Snapshot of nDoRunCalls taken inside DoNewRound() - see
   * GOTestSoundTaskBase::TestNewRoundWaitsForInFlightRun */
  std::atomic<int> nDoRunCallsAsOfLastNewRound{-1};
  /**
   * Stands in for GOSoundWindchestTask::m_volume, published by DoRun() and
   * read back through GetPayloadLazily(). Atomic, unlike m_volume: the base
   * Run() takes m_mutex non-blocking (try-lock), so a caller whose own Run()
   * call loses the race - or whose DoRun() may legitimately return false for
   * the whole round - can read this concurrently with another thread's
   * DoRun() still writing it. GOSoundWindchestTask::Run() instead blocks on
   * its own mutex until done, which is what lets its m_volume stay plain -
   * this double intentionally exercises the weaker default protocol, so the
   * payload itself must supply the safety that guarantee would otherwise
   * give it.
   */
  std::atomic<float> payload{0};

  /**
   * @param priority forwarded to GOSoundTaskBase
   * @param isRepeatable forwarded to GOSoundTaskBase
   */
  GOSoundTaskTestImpl(
    TaskPriority priority = PRIORITY_AUDIOGROUP, bool isRepeatable = false)
    : GOSoundTaskBase(priority, isRepeatable) {}

  /** Records concurrency/call counts, optionally sleeps, then returns
   * doRunResult */
  bool DoRun(GOSchedulerThread *pThread) override;

  /** Records that a new round started and snapshots nDoRunCalls */
  void DoNewRound() override;

  /** @return the protected m_IsToComplete, for assertions */
  bool GetIsToComplete() const { return m_IsToComplete.load(); }

  /**
   * Reads the payload the way GOSoundWindchestTask::GetVolume() reads
   * m_volume: lazily runs the task, then immediately reads what the run
   * published. A caller that returns from Run() before the round's result is
   * ready observes a stale payload here.
   * @return the payload published by the current round
   */
  float GetPayloadLazily();
};

#endif /* GOSOUNDTASKTESTIMPL_H */
