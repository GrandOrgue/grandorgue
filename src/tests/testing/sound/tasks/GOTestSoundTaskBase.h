/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTASKBASE_H
#define GOTESTSOUNDTASKBASE_H

#include <string>

#include "GOTest.h"

class GOTestSoundTaskBase : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A freshly constructed task reports the constructor arguments back and
   * has not yet processed a round. */
  void TestInitialState();

  /** Run() calls DoRun() once and, once DoRun() returns true, further
   * Run() calls within the same round are no-ops. */
  void TestRunCallsDoRunOnce();

  /** While DoRun() returns false, every Run() call invokes DoRun() again
   * and the task never becomes done (the GOSoundTouchTask protocol). */
  void TestRunRepeatsUntilDoRunReturnsTrue();

  /** CompleteRound() sets m_IsToComplete and keeps calling Run() until the
   * task is actually done, even if DoRun() keeps returning false. */
  void TestCompleteRoundSetsFlagAndFinishes();

  /** NewRound() resets the round state and m_IsToComplete and calls
   * DoNewRound() exactly once. */
  void TestNewRoundResetsState();

  /** DiscardContent() has the same observable effect as NewRound(). */
  void TestDiscardContentCallsNewRound();

  /** Several threads calling Run() concurrently on the same task must
   * still call DoRun() exactly once, never overlapping. */
  void TestConcurrentRunExecutesDoRunExactlyOnce();

  /** Same as above but with DoRun() returning false: several threads may
   * each contribute a DoRun() call, but never two at once. */
  void TestConcurrentRunWithIncompleteDoRun();

  /** A thread calling CompleteRound() while another keeps calling Run()
   * never overlaps with it in DoRun() and leaves the task done. */
  void TestConcurrentCompleteRoundAndRun();

  /** NewRound() must not observe/reset the round while a DoRun() started
   * by a different thread is still in flight. */
  void TestNewRoundWaitsForInFlightRun();

  /** Across many rounds, driven by threads racing Run()/CompleteRound(),
   * the task must never report done without DoRun() having run. */
  void TestNoTornStateAcrossThreads();

  /** In the cooperative protocol (see CooperativeTaskImpl), many threads
   * joining a round contribute exactly once each, with no work lost or
   * duplicated, and the round finishes exactly once. */
  void TestCooperativeReachesDoneExactlyOnce();

  /** When there is less work than threads, the threads that find nothing
   * left neither lose nor duplicate the work already claimed. */
  void TestCooperativeThreadWithNoWorkExitsEarly();

  /** A thread waiting on the round's condition variable is not woken until
   * the round has actually finished. */
  void TestCooperativeWaitOrStopBlocksUntilDone();

  /** NewRound() lets the cooperative protocol run a fresh, independent
   * round without leaking state from the previous one. */
  void TestCooperativeNewRoundAllowsFreshRound();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTASKBASE_H */
