/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTaskTestImpl.h"

#include <chrono>
#include <thread>

bool GOSoundTaskTestImpl::DoRun(GOSchedulerThread *pThread) {
  // widen the race window and record the peak concurrency observed inside
  // the section that DoRun() relies on being exclusive
  const int nConcurrent = nConcurrentDoRunCalls.fetch_add(1) + 1;
  int prevMax = nMaxConcurrentDoRunCalls.load();

  while (
    nConcurrent > prevMax
    && !nMaxConcurrentDoRunCalls.compare_exchange_weak(prevMax, nConcurrent))
    ;
  const int nWorkMicroseconds = doRunWorkMicroseconds.load();

  if (doRunSleepMicroseconds.load() > 0)
    std::this_thread::sleep_for(
      std::chrono::microseconds(doRunSleepMicroseconds.load()));
  if (nWorkMicroseconds > 0) {
    // busy-wait rather than sleep: this stands in for real per-round work,
    // and a sleeping lock holder would measure wake-up granularity instead
    // of the cost of the round protocol
    const auto workEnd = std::chrono::steady_clock::now()
      + std::chrono::microseconds(nWorkMicroseconds);

    while (std::chrono::steady_clock::now() < workEnd)
      ;
  }
  const int callN = nDoRunCalls.fetch_add(1) + 1;
  payload.store(static_cast<float>(callN));
  nConcurrentDoRunCalls.fetch_sub(1);
  return doRunResult.load();
}

float GOSoundTaskTestImpl::GetPayloadLazily() {
  if (!IsDone())
    Run();
  return payload.load();
}

void GOSoundTaskTestImpl::DoNewRound() {
  nDoRunCallsAsOfLastNewRound.store(nDoRunCalls.load());
  nDoNewRoundCalls.fetch_add(1);
  // 0 is TestPerfLazyPrerequisiteAccess's "this round never published"
  // marker; without resetting it here, a regression in any round after the
  // first would go undetected once payload was ever positive
  payload.store(0);
}
