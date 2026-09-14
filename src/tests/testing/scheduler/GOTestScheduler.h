/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSCHEDULER_H
#define GOTESTSCHEDULER_H

#include <string>

#include "GOTest.h"

class GOTestScheduler : public GOTest {
private:
  static const std::string TEST_NAME;

  /** An idle scheduler thread is woken and runs newly available work after
   * PauseGivingWork()/WaitForIdle() followed by
   * NewRound()/ResumeGivingWork()/Wakeup() - the same sequence
   * StopEngine()/StartEngine() perform. */
  void TestResumeThenWakeupRunsIdleThreadsWork();

  /** Repeating that pause/resume/wakeup cycle many times on one long-lived
   * thread never loses a wakeup - regression coverage for the lost-wakeup
   * race in GOSchedulerThread::Wakeup()/Entry(). */
  void TestPauseResumeWakeupCyclesAreNotLost();

  /** Delete() (MarkForStop() + Wakeup() + join) always returns, even while
   * another thread is concurrently hammering Wakeup() - regression coverage
   * for the same race dropping the shutdown wakeup and hanging the join. */
  void TestDeleteReturnsWhenRacingWakeup();

  /** std::atomic<bool>::wait(old) returns immediately if the value already
   * differs from old at call time, with no notify() call following it at
   * all. This is the exact guarantee GOSchedulerThread::Entry()'s park
   * relies on to close the race a condition_variable-based wait cannot: a
   * condition_variable::wait_for() only reacts to a notify that arrives
   * after the wait call has begun, so a notify_one() issued earlier (while
   * nothing was parked yet) is simply lost - recoverable only via an
   * unrelated timeout, which is the ~1s latency defect this whole
   * mechanism replaced. Deliberately does not attempt to provoke this race
   * through GOSchedulerThread/Wakeup() end-to-end: hammering Wakeup() in a
   * tight loop was tried and does not reproduce it even against the old,
   * defective implementation - a lost notify_one() is immediately masked
   * by the next one arriving microseconds later once the worker has parked,
   * since real callers (one Wakeup() per audio period) never retry that
   * fast. Testing the primitive directly, with a single store before the
   * single wait() call and no notify ever following it, is what actually
   * isolates the guarantee. */
  void TestAtomicWaitObservesStoreThatPrecedesIt();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSCHEDULER_H */
