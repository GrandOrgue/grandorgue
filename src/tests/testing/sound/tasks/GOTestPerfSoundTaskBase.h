/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDTASKBASE_H
#define GOTESTPERFSOUNDTASKBASE_H

#include <string>
#include <vector>

#include "GOTest.h"

class GOTestPerfSoundTaskBase : public GOTest {
private:
  static const std::string TEST_NAME;

  std::vector<std::string> m_failedTests;

  /** A single thread repeatedly calling Run() on an already-done,
   * repeatable task: the hot path taken by a worker thread that pulls a
   * task it has nothing left to do for this round. */
  void TestPerfUncontendedRunThroughput();

  /** Several threads repeatedly racing Run() on the same non-cooperative
   * task, each round doing a small amount of real work in DoRun(): measures
   * per-round latency and checks it does not blow up with thread count. */
  void TestPerfContendedRunLatency();

  /** Several threads repeatedly racing Run() on the same cooperative task
   * (see CooperativeTaskImpl in GOTestSoundTaskBase.cpp): measures
   * aggregate throughput of units of work processed per second. */
  void TestPerfCooperativeThroughput();

  /**
   * Cost of the round protocol alone - the m_mutex acquisitions in Run() and
   * NewRound() - with DoRun() doing nothing at all. The same loop is timed
   * twice, once with Run() in the body and once without, and only the
   * difference is reported, so that barrier and thread-scheduling overhead
   * is subtracted instead of diluting the result. This is the number a
   * lock-free rewrite of the protocol has to move.
   */
  void TestPerfRoundProtocolCost();

  /**
   * The shape of GOSoundOrganEngine::NextPeriod(): one thread standing in
   * for the audio thread calls CompleteRound() then NewRound() in a loop
   * while worker threads race Run() on the same task. Reports that one
   * thread's own latency - the priority inversion cost - as mean and
   * maximum, both against the 333.33 us budget of a 32 frame / 96000 Hz
   * period, and thresholds them separately: audio drops out on the tail of
   * the distribution, not on the average.
   */
  void TestPerfNextPeriodLatency();

  /**
   * The lazy-prerequisite path: several threads reading a not-yet-done task
   * the way GOSoundWindchestTask::GetVolume() reads m_volume. Run in both
   * protocols - DoRun() returning true, and DoRun() always returning false
   * as GOSoundTouchTask does, where the task never becomes done within a
   * round and every thread that loses the race must go and do its own share.
   */
  void TestPerfLazyPrerequisiteAccess();

public:
  GOTestPerfSoundTaskBase() : GOTest(GOTest::PERF) {}
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTPERFSOUNDTASKBASE_H */
