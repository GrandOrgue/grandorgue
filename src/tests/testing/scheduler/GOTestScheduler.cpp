/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestScheduler.h"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include "scheduler/GOScheduler.h"
#include "scheduler/GOSchedulerTask.h"
#include "scheduler/GOSchedulerThread.h"

#include "GOTestScope.h"

const std::string GOTestScheduler::TEST_NAME = "GOTestScheduler";

namespace {

// Signals wasRun once actually Run() by a scheduler thread, so a test can
// prove a task was picked up on its own - not driven by anything else -
// within a bounded wait.
class SignalingTask : public GOSchedulerTask {
public:
  std::atomic_bool wasRun{false};

  unsigned GetPriority() const override { return 0; }
  unsigned GetCost() const override { return 0; }
  bool IsRepeatable() const override { return false; }
  bool IsEmpty() const override { return true; }
  void Run(GOSchedulerThread * = nullptr) override { wasRun.store(true); }
  void CompleteRound() override {}
  void NewRound() override {}
  void DiscardContent() override {}
};

// Calls Delete() on the wrapped thread from its destructor, so a GOAssert
// failure thrown out of a soak loop still stops the thread instead of
// leaking a running std::thread - whose destructor would otherwise call
// std::terminate() and crash the whole test process instead of reporting
// the failure.
class SchedulerThreadGuard {
private:
  GOSchedulerThread &r_Thread;

public:
  explicit SchedulerThreadGuard(GOSchedulerThread &thread) : r_Thread(thread) {}
  ~SchedulerThreadGuard() { r_Thread.Delete(); }
};

} // namespace

void GOTestScheduler::TestResumeThenWakeupRunsIdleThreadsWork() {
  SignalingTask task;
  GOScheduler scheduler;
  GOSchedulerThread thread(&scheduler);

  thread.Run();
  thread.WaitForIdle(); // parked - nothing registered yet

  scheduler.Add(&task);

  // Mirrors GOSoundOrganEngine::StopEngine() then the fixed StartEngine():
  // pause, wait for the thread to park, then make a fresh round available
  // and explicitly wake it. Regression coverage for the "wake aux workers on
  // resume" fix: without the final Wakeup(), the thread would stay parked on
  // its condition variable forever - GetNextTask() returning real work is
  // not, by itself, enough to bring an idle thread back.
  scheduler.PauseGivingWork();
  thread.WaitForIdle();

  scheduler.NewRound();
  scheduler.ResumeGivingWork();
  thread.Wakeup();

  const auto deadline
    = std::chrono::steady_clock::now() + std::chrono::seconds(1);

  while (!task.wasRun.load() && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();

  thread.Delete();

  GOAssert(
    task.wasRun.load(),
    "an idle scheduler thread must be woken and run newly available work "
    "after PauseGivingWork()/WaitForIdle() followed by "
    "NewRound()/ResumeGivingWork()/Wakeup() - the same sequence "
    "StopEngine()/StartEngine() perform");
}

void GOTestScheduler::TestPauseResumeWakeupCyclesAreNotLost() {
  SignalingTask task;
  GOScheduler scheduler;
  GOSchedulerThread thread(&scheduler);
  SchedulerThreadGuard guard(thread);

  thread.Run();
  thread.WaitForIdle();
  scheduler.Add(&task);

  for (unsigned cycleI = 0; cycleI < 200; cycleI++) {
    task.wasRun.store(false);
    scheduler.PauseGivingWork();
    thread.WaitForIdle();
    scheduler.NewRound();
    scheduler.ResumeGivingWork();
    thread.Wakeup();

    const auto deadline
      = std::chrono::steady_clock::now() + std::chrono::seconds(1);

    while (!task.wasRun.load() && std::chrono::steady_clock::now() < deadline)
      std::this_thread::yield();

    GOAssert(
      task.wasRun.load(),
      "cycle " + std::to_string(cycleI)
        + ": a wakeup was lost after PauseGivingWork()/WaitForIdle() "
          "followed by NewRound()/ResumeGivingWork()/Wakeup() - this is the "
          "GOSchedulerThread::Wakeup()/Entry() lost-wakeup race");
  }
}

void GOTestScheduler::TestDeleteReturnsWhenRacingWakeup() {
  for (unsigned iterationI = 0; iterationI < 50; iterationI++) {
    // Heap-allocated and, on a regression, deliberately leaked: if Delete()
    // never returns, the racing threads below must not be joined (that
    // would just relocate the hang), so ownership of these objects cannot
    // be transferred back to a scope that destroys them.
    auto *pScheduler = new GOScheduler();
    auto *pThread = new GOSchedulerThread(pScheduler);
    auto *pIsHammering = new std::atomic_bool(true);
    auto *pIsDeleted = new std::atomic_bool(false);

    pThread->Run();

    std::thread hammerThread([pThread, pIsHammering] {
      while (pIsHammering->load())
        pThread->Wakeup();
    });
    std::thread deleteThread([pThread, pIsDeleted] {
      pThread->Delete();
      pIsDeleted->store(true);
    });

    const auto deadline
      = std::chrono::steady_clock::now() + std::chrono::seconds(2);

    while (!pIsDeleted->load() && std::chrono::steady_clock::now() < deadline)
      std::this_thread::yield();

    const bool wasDeleted = pIsDeleted->load();

    pIsHammering->store(false);
    hammerThread.join();
    if (wasDeleted) {
      deleteThread.join();
      delete pThread;
      delete pScheduler;
      delete pIsHammering;
      delete pIsDeleted;
    } else
      deleteThread.detach();

    GOAssert(
      wasDeleted,
      "iteration " + std::to_string(iterationI)
        + ": GOSchedulerThread::Delete() must return within the deadline "
          "even while another thread concurrently hammers Wakeup()");
  }
}

void GOTestScheduler::TestAtomicWaitObservesStoreThatPrecedesIt() {
  // Comfortably above any latency this in-process, uncontended wait() call
  // should ever show, and comfortably below the ~1s THREADING_WAIT_TIMEOUT
  // signature a condition_variable-based fallback would show instead.
  static const auto MAX_ACCEPTABLE_LATENCY = std::chrono::milliseconds(50);

  for (unsigned iterationI = 0; iterationI < 1000; iterationI++) {
    std::atomic_bool flag{false};

    flag.store(true); // simulates Wakeup() firing before the wait call

    const auto start = std::chrono::steady_clock::now();

    flag.wait(false); // must return immediately - no notify() ever follows

    const auto elapsed = std::chrono::steady_clock::now() - start;

    GOAssert(
      elapsed < MAX_ACCEPTABLE_LATENCY,
      "iteration " + std::to_string(iterationI) + ": atomic<bool>::wait(false) "
        + "took "
        + std::to_string(
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
            .count())
        + "ms to return after the value was already true before the call, "
          "with no notify() ever issued - it must re-check the current "
          "value and return immediately instead of behaving like a "
          "condition_variable, which would block until an unrelated "
          "timeout");
  }
}

void GOTestScheduler::run() {
  GO_RUN_TEST(TestResumeThenWakeupRunsIdleThreadsWork())
  GO_RUN_TEST(TestPauseResumeWakeupCyclesAreNotLost())
  GO_RUN_TEST(TestDeleteReturnsWhenRacingWakeup())
  GO_RUN_TEST(TestAtomicWaitObservesStoreThatPrecedesIt())
}
