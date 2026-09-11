/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTaskBase.h"

#include <atomic>
#include <chrono>
#include <format>
#include <memory>
#include <thread>
#include <vector>

#include "sound/tasks/GOSoundTaskBase.h"

#include "GOSoundCooperativeTaskTestImpl.h"
#include "GOSoundTaskTestImpl.h"
#include "GOTestScope.h"

const std::string GOTestSoundTaskBase::TEST_NAME = "GOTestSoundTaskBase";

// Starts nThreads running body concurrently and joins all of them
static void run_on_threads(unsigned nThreads, std::function<void()> body) {
  std::vector<std::thread> threads;

  for (unsigned threadI = 0; threadI < nThreads; threadI++)
    threads.emplace_back(body);
  for (std::thread &thread : threads)
    thread.join();
}

void GOTestSoundTaskBase::TestInitialState() {
  GOSoundTaskTestImpl task(GOSoundTaskBase::PRIORITY_TREMULANT, true);

  GOAssert(!task.IsDone(), "a fresh task must not be done");
  GOAssert(
    task.GetPriority() == GOSoundTaskBase::PRIORITY_TREMULANT,
    "GetPriority() must return the value passed to the constructor");
  GOAssert(
    task.IsRepeatable(), "IsRepeatable() must return the constructor value");
  GOAssert(task.GetCost() == 0, "the default GetCost() must be 0");
}

void GOTestSoundTaskBase::TestRunCallsDoRunOnce() {
  GOSoundTaskTestImpl task;

  task.Run();
  GOAssert(task.IsDone(), "the task must be done after DoRun() returns true");
  GOAssert(
    task.nDoRunCalls.load() == 1, "DoRun() must have been called exactly once");

  task.Run();
  task.Run();
  GOAssert(
    task.nDoRunCalls.load() == 1,
    "further Run() calls within the same round must not call DoRun() again");
}

void GOTestSoundTaskBase::TestRunRepeatsUntilDoRunReturnsTrue() {
  GOSoundTaskTestImpl task;

  task.doRunResult.store(false);
  task.Run();
  task.Run();
  task.Run();
  GOAssert(
    !task.IsDone(),
    "the task must stay not-done while DoRun() keeps returning false");
  GOAssert(
    task.nDoRunCalls.load() == 3,
    "DoRun() must be called again on every Run() while not done");
}

void GOTestSoundTaskBase::TestCompleteRoundSetsFlagAndFinishes() {
  GOSoundTaskTestImpl task;

  task.doRunResult.store(false);
  task.CompleteRound();
  GOAssert(task.GetIsToComplete(), "CompleteRound() must set m_IsToComplete");
  GOAssert(
    task.nDoRunCalls.load() == 1,
    "CompleteRound() must call Run()/DoRun() exactly once");
  GOAssert(
    !task.IsDone(),
    "CompleteRound() calling Run() once must not force IsDone() true when "
    "DoRun() itself returned false - a DoRun() that cannot guarantee "
    "finishing in one call must override CompleteRound() instead");
}

void GOTestSoundTaskBase::TestNewRoundResetsState() {
  GOSoundTaskTestImpl task;

  task.Run();
  task.CompleteRound();
  task.NewRound();
  GOAssert(!task.IsDone(), "NewRound() must reset the round state");
  GOAssert(!task.GetIsToComplete(), "NewRound() must clear m_IsToComplete");
  GOAssert(
    task.nDoNewRoundCalls.load() == 1, "DoNewRound() must be called once");
}

void GOTestSoundTaskBase::TestDiscardContentCallsNewRound() {
  GOSoundTaskTestImpl task;

  task.Run();
  task.DiscardContent();
  GOAssert(!task.IsDone(), "DiscardContent() must reset the round state");
  GOAssert(
    task.nDoNewRoundCalls.load() == 1,
    "DiscardContent() must call DoNewRound() exactly like NewRound()");
}

static constexpr unsigned N_STRESS_ITERATIONS = 200;
static constexpr unsigned N_STRESS_THREADS = 8;

void GOTestSoundTaskBase::TestConcurrentRunExecutesDoRunExactlyOnce() {
  for (unsigned iterI = 0; iterI < N_STRESS_ITERATIONS; iterI++) {
    GOSoundTaskTestImpl task;

    run_on_threads(N_STRESS_THREADS, [&task]() { task.Run(); });
    GOAssert(
      task.nDoRunCalls.load() == 1,
      std::format(
        "iteration {}: DoRun() must run exactly once for {} concurrent "
        "Run() calls",
        iterI,
        N_STRESS_THREADS));
    GOAssert(
      task.nMaxConcurrentDoRunCalls.load() == 1,
      std::format(
        "iteration {}: m_mutex must exclude concurrent DoRun() execution",
        iterI));
    GOAssert(
      task.IsDone(), std::format("iteration {}: task must end up done", iterI));
  }
}

void GOTestSoundTaskBase::TestConcurrentRunWithIncompleteDoRun() {
  for (unsigned iterI = 0; iterI < N_STRESS_ITERATIONS; iterI++) {
    GOSoundTaskTestImpl task;

    task.doRunResult.store(false);
    run_on_threads(N_STRESS_THREADS, [&task]() { task.Run(); });
    GOAssert(
      !task.IsDone(),
      std::format(
        "iteration {}: task must never become done while DoRun() returns "
        "false",
        iterI));
    GOAssert(
      task.nMaxConcurrentDoRunCalls.load() == 1,
      std::format(
        "iteration {}: m_mutex must exclude concurrent DoRun() execution "
        "even when incremental",
        iterI));
  }
}

void GOTestSoundTaskBase::TestConcurrentCompleteRoundAndRun() {
  for (unsigned iterI = 0; iterI < N_STRESS_ITERATIONS; iterI++) {
    GOSoundTaskTestImpl task;
    std::atomic<bool> isRunning{true};

    std::thread runner([&task, &isRunning]() {
      while (isRunning.load())
        task.Run();
    });
    std::this_thread::sleep_for(std::chrono::microseconds(50));
    task.CompleteRound();
    isRunning.store(false);
    runner.join();

    GOAssert(
      task.IsDone(),
      std::format(
        "iteration {}: CompleteRound() must finish the round", iterI));
    GOAssert(
      task.nMaxConcurrentDoRunCalls.load() == 1,
      std::format(
        "iteration {}: CompleteRound() and Run() must not run DoRun() "
        "concurrently",
        iterI));
  }
}

void GOTestSoundTaskBase::TestNewRoundWaitsForInFlightRun() {
  for (unsigned iterI = 0; iterI < N_STRESS_ITERATIONS; iterI++) {
    GOSoundTaskTestImpl task;

    task.doRunSleepMicroseconds.store(5000);

    std::thread runner([&task]() { task.Run(); });

    // wait until DoRun() has actually entered (not just Run() called) before
    // racing NewRound() in: nConcurrentDoRunCalls is incremented at the very
    // start of DoRun(), before its sleep
    while (task.nConcurrentDoRunCalls.load() == 0)
      ;
    task.NewRound();
    runner.join();

    GOAssert(
      task.nDoRunCallsAsOfLastNewRound.load() == task.nDoRunCalls.load(),
      std::format(
        "iteration {}: NewRound() must not run concurrently with an "
        "in-flight DoRun()",
        iterI));
  }
}

void GOTestSoundTaskBase::TestNoTornStateAcrossThreads() {
  GOSoundTaskTestImpl task;

  for (unsigned roundI = 0; roundI < N_STRESS_ITERATIONS; roundI++) {
    const int nCallsBeforeRound = task.nDoRunCalls.load();
    std::vector<std::thread> threads;

    for (unsigned threadI = 0; threadI < N_STRESS_THREADS; threadI++)
      threads.emplace_back([&task, threadI]() {
        if (threadI % 2 == 0)
          task.Run();
        else
          task.CompleteRound();
      });
    for (std::thread &thread : threads)
      thread.join();

    const bool isDone = task.IsDone();
    const int nCallsThisRound = task.nDoRunCalls.load() - nCallsBeforeRound;

    GOAssert(
      !(isDone && nCallsThisRound == 0),
      std::format(
        "round {}: task is done but DoRun() was never called this round",
        roundI));
    task.NewRound();
  }
}

void GOTestSoundTaskBase::TestCooperativeReachesDoneExactlyOnce() {
  GOSoundCooperativeTaskTestImpl task;

  task.SetWorkItems(1000);
  run_on_threads(N_STRESS_THREADS, [&task]() { task.Run(); });

  GOAssert(
    task.nFirstTransitions.load() == 1,
    "exactly one thread must claim the round");
  GOAssert(
    task.nCopyTransitions.load() == 1,
    "exactly one thread must store the first partial result");
  GOAssert(
    task.nLastThreadFinishes.load() == 1,
    "exactly one thread must finish the round");
  GOAssert(
    task.nSharedValue.load() == 1000,
    "every unit of work must be accounted for exactly once");
  GOAssert(task.IsDone(), "the round must end up done");
}

void GOTestSoundTaskBase::TestCooperativeThreadWithNoWorkExitsEarly() {
  GOSoundCooperativeTaskTestImpl task;

  task.SetWorkItems(1);
  run_on_threads(N_STRESS_THREADS, [&task]() { task.Run(); });

  GOAssert(
    task.nSharedValue.load() == 1,
    "the single unit of work must be neither lost nor duplicated");
  GOAssert(
    task.nLastThreadFinishes.load() == 1,
    "threads with no work left must not prevent the round from finishing");
  GOAssert(task.IsDone(), "the round must end up done");
}

void GOTestSoundTaskBase::TestCooperativeWaitOrStopBlocksUntilDone() {
  GOSoundCooperativeTaskTestImpl task;

  task.SetWorkItems(200);

  std::thread runner([&task]() { task.Run(); });
  std::thread waiter([&task]() { task.WaitUntilDone(); });

  runner.join();
  waiter.join();

  GOAssert(
    task.nLastThreadFinishes.load() == 1,
    "the waiter must not return before the round actually finishes");
  GOAssert(task.IsDone(), "the round must be done once the waiter returns");
}

void GOTestSoundTaskBase::TestCooperativeNewRoundAllowsFreshRound() {
  GOSoundCooperativeTaskTestImpl task;

  task.SetWorkItems(500);
  run_on_threads(N_STRESS_THREADS, [&task]() { task.Run(); });
  task.NewRound();

  task.SetWorkItems(700);
  run_on_threads(N_STRESS_THREADS, [&task]() { task.Run(); });

  GOAssert(
    task.nFirstTransitions.load() == 2,
    "each round must be claimed exactly once");
  GOAssert(
    task.nCopyTransitions.load() == 2,
    "each round must produce exactly one first partial result");
  GOAssert(
    task.nLastThreadFinishes.load() == 2,
    "each round must finish exactly once");
  GOAssert(
    task.nSharedValue.load() == 700,
    "the second round's result must not be mixed with the first round's");
}

void GOTestSoundTaskBase::TestEnsureBufferReadyDoesNotHangOnRaceWithNewRound() {
  // Owned via unique_ptr, not a stack variable: on a regression, the waiter
  // thread below is stuck forever and must not be joined (that would just
  // relocate the hang into this test - see TestDeleteReturnsWhenRacingWakeup
  // in GOTestScheduler.cpp for the same rationale), so ownership has to be
  // releasable into a deliberate, documented leak rather than freed by a
  // destructor the detached thread could still be using.
  auto pTask = std::make_unique<GOSoundCooperativeTaskTestImpl>();
  auto pIsNewRoundDone = std::make_unique<std::atomic<bool>>(false);
  auto pIsFinished = std::make_unique<std::atomic<bool>>(false);

  GOSoundCooperativeTaskTestImpl *const pTaskRaw = pTask.get();
  std::atomic<bool> *const pIsNewRoundDoneRaw = pIsNewRoundDone.get();
  std::atomic<bool> *const pIsFinishedRaw = pIsFinished.get();

  pTaskRaw->SetWorkItems(0);
  pTaskRaw->Run(); // no work, no participants -> RUN_STATE_DONE immediately

  GOAssert(
    pTaskRaw->IsDone(),
    "precondition: a work-free round must finish synchronously so the "
    "later EnsureBufferReady() call's own Run() is the fast no-op the race "
    "needs");

  std::thread waiter([pTaskRaw, pIsNewRoundDoneRaw, pIsFinishedRaw]() {
    pTaskRaw->EnsureBufferReady(nullptr, [pTaskRaw, pIsNewRoundDoneRaw]() {
      // Lands exactly in the gap between EnsureBufferReady()'s Run() call
      // (which just saw RUN_STATE_DONE and returned instantly) and its
      // separate, later m_RunState re-check - deterministically, instead of
      // relying on the ~2% chance this raced by luck against the real
      // GOSoundGroupTask in production.
      std::thread racer([pTaskRaw]() {
        // Mirrors GOScheduler::NewRound() advancing GOScheduler::m_RoundCounter
        // alongside the task-state reset it invalidates.
        pTaskRaw->roundCounter.AdvanceRound();
        pTaskRaw->NewRound();
      });

      racer.join();
      pIsNewRoundDoneRaw->store(true);
    });
    pIsFinishedRaw->store(true);
  });

  const auto deadline
    = std::chrono::steady_clock::now() + std::chrono::seconds(2);

  while (!pIsFinishedRaw->load() && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();

  const bool didFinish = pIsFinishedRaw->load();

  if (didFinish) {
    waiter.join();
  } else {
    // The waiter is parked in EnsureBufferReady() forever, still touching
    // *pTask/*pIsNewRoundDone/*pIsFinished through the raw pointers above -
    // release them from unique_ptr instead of letting the destructors run.
    waiter.detach();
    pTask.release();
    pIsNewRoundDone.release();
    pIsFinished.release();
  }

  GOAssert(
    pIsNewRoundDoneRaw->load(),
    "precondition: the racing NewRound() must have completed before "
    "EnsureBufferReady() re-checks m_RunState");
  GOAssert(
    didFinish,
    "EnsureBufferReady() must not wait forever on a round that a "
    "concurrent NewRound() reset between its own Run() call and its "
    "separate m_RunState re-check - see the TOCTOU race documented on "
    "GOSoundGroupTask::EnsureBufferReady()");
}

void GOTestSoundTaskBase::run() {
  GO_RUN_TEST(TestInitialState())
  GO_RUN_TEST(TestRunCallsDoRunOnce())
  GO_RUN_TEST(TestRunRepeatsUntilDoRunReturnsTrue())
  GO_RUN_TEST(TestCompleteRoundSetsFlagAndFinishes())
  GO_RUN_TEST(TestNewRoundResetsState())
  GO_RUN_TEST(TestDiscardContentCallsNewRound())

  GO_RUN_TEST(TestConcurrentRunExecutesDoRunExactlyOnce())
  GO_RUN_TEST(TestConcurrentRunWithIncompleteDoRun())
  GO_RUN_TEST(TestConcurrentCompleteRoundAndRun())
  GO_RUN_TEST(TestNewRoundWaitsForInFlightRun())
  GO_RUN_TEST(TestNoTornStateAcrossThreads())

  GO_RUN_TEST(TestCooperativeReachesDoneExactlyOnce())
  GO_RUN_TEST(TestCooperativeThreadWithNoWorkExitsEarly())
  GO_RUN_TEST(TestCooperativeWaitOrStopBlocksUntilDone())
  GO_RUN_TEST(TestCooperativeNewRoundAllowsFreshRound())
  GO_RUN_TEST(TestEnsureBufferReadyDoesNotHangOnRaceWithNewRound())
}
