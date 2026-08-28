/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundTaskBase.h"

#include <atomic>
#include <barrier>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

#include "GOSoundCooperativeTaskTestImpl.h"
#include "GOSoundTaskTestImpl.h"

const std::string GOTestPerfSoundTaskBase::TEST_NAME
  = "GOTestPerfSoundTaskBase";

// The audio period this suite is calibrated against: 32 frames at 96000 Hz,
// i.e. the tightest real-time budget the round protocol has to fit into.
// Every threshold below that can be expressed as a fraction of a period is
// derived from these two numbers rather than from an arbitrary margin, so
// that a failure means "this would not keep up with the audio device"
// instead of "this got slower than it was on some developer's machine".
static constexpr double N_FRAMES_PER_BUFFER = 32.0;
static constexpr double SAMPLE_RATE = 96000.0;
/** 333.33 us - the wall-clock budget for one round */
static constexpr double AUDIO_PERIOD_MICROSECONDS
  = N_FRAMES_PER_BUFFER * 1'000'000.0 / SAMPLE_RATE;
/** 3000 - the number of rounds per second the engine must sustain */
static constexpr double AUDIO_PERIODS_PER_SECOND
  = SAMPLE_RATE / N_FRAMES_PER_BUFFER;

// Minimum acceptable throughput of the uncontended hot path: a repeatable
// task that is already done for this round and is being repeatedly polled
// by a worker thread with nothing better to do, as GOSchedulerThread does
// while looking for work. Calibrated from the CI history of this test on
// GitHub Actions' shared ubuntu-latest runners (github.com/oleg68/
// GrandOrgue-official, Build workflow, "tests (perf, ...)" jobs, 16 Debug/
// Release runs between 2026-08-04 and 2026-08-28), separately per build
// type since they don't share a noise floor: observed 359M-493M runs/sec on
// Debug (worst 359M) and 375M-435M on Release (worst 375M), each threshold
// set 10% below its own worst case for run-to-run variance on shared
// hardware.
#ifdef NDEBUG
static constexpr double MIN_UNCONTENDED_RUNS_PER_SECOND = 335'000'000.0;
#else
static constexpr double MIN_UNCONTENDED_RUNS_PER_SECOND = 320'000'000.0;
#endif

// Minimum acceptable round rate for several threads racing Run() on the same
// task with ~10us of real work each. This is the real-time requirement
// itself: at 32 frames / 96000 Hz the engine must complete 3000 rounds per
// second. Local Debug runs give 30383-96810 rounds/sec (nThreads=8..1), i.e.
// 10-32x headroom over the requirement.
static constexpr double MIN_CONTENDED_ROUNDS_PER_SECOND
  = AUDIO_PERIODS_PER_SECOND;

// Minimum acceptable aggregate throughput of the cooperative protocol, now
// that each thread works its own quota instead of draining one shared
// counter. Calibrated from 6 local Debug runs: worst observed was ~532M
// items/sec (nThreads=1), halved for a 2x margin. With the shared-counter
// artifact gone, throughput scales up with thread count (~740M at 1 thread
// to ~3.9G at 8) rather than collapsing, so the worst case is now the
// single-threaded one.
// TODO: recalibrate once this test has run on CI hardware.
static constexpr double MIN_COOPERATIVE_ITEMS_PER_SECOND = 250'000'000.0;

// Maximum acceptable cost of the round protocol itself - the m_mutex
// acquisitions in Run() and NewRound() - with barrier and thread scheduling
// subtracted out via a control run. Budgeted at 10% of one audio period:
// the handshake is pure overhead, so it must stay a small fraction of the
// time available for actually producing sound. Local Debug runs give
// 90-3455 ns (nThreads=1..8) against this 33333 ns budget.
// A negative measurement means the protocol is below the harness's noise
// floor, which passes.
static constexpr double MAX_PROTOCOL_NANOSECONDS_PER_ROUND
  = AUDIO_PERIOD_MICROSECONDS * 1'000.0 / 10.0;

// Maximum acceptable mean latency of one CompleteRound() + NewRound() pair
// on the thread standing in for the audio thread, while workers race Run().
// Budgeted at half an audio period: this handshake is only one part of what
// NextPeriod() has to fit into 333.33 us. Local Debug runs give 5.6-6.0 us,
// i.e. essentially just the 5 us of simulated work: the handshake itself is
// close to free once the lock holder is not sleeping inside the critical
// section (see doRunWorkMicroseconds vs doRunSleepMicroseconds).
static constexpr double MAX_NEXT_PERIOD_MEAN_MICROSECONDS
  = AUDIO_PERIOD_MICROSECONDS / 2.0;

// Maximum acceptable worst-case latency of that same pair, thresholded
// separately because audio drops out on the tail, not on the average.
// Local Debug runs peak at 13-612 us, mostly 15-100 us, while the mean stays
// flat at 5.6 us - that shape is OS scheduling jitter of the measuring
// thread rather than contention on m_mutex. Budgeted at 10 periods: still
// ~5x above the worst local observation, so it catches an RT thread blocked
// for several periods without flapping on a loaded CI machine.
static constexpr double MAX_NEXT_PERIOD_PEAK_MICROSECONDS
  = AUDIO_PERIOD_MICROSECONDS * 10.0;

// Minimum acceptable round rate on the lazy-prerequisite path
// (GOSoundWindchestTask::GetVolume() shape) with ~5us of work per DoRun().
// Unlike MIN_CONTENDED_ROUNDS_PER_SECOND this is NOT the 3000/sec real-time
// figure: the timed loop includes a full barrier per round, which dominates.
// Calibrated from 6 local Debug runs: worst observed was ~13986 rounds/sec
// (doneWithinRound=false, nThreads=8), halved for a 2x margin.
// TODO: recalibrate once this test has run on CI hardware.
static constexpr double MIN_LAZY_ROUNDS_PER_SECOND = 7'000.0;

/**
 * Times nRounds barrier-synchronised rounds across nThreads threads.
 * @param nThreads number of threads taking part in every round
 * @param nRounds number of rounds to time
 * @param pTask the task to Run() and NewRound() each round; nullptr times
 *   the bare barrier loop, which is the control measurement to subtract
 * @return elapsed time in seconds
 */
static double measure_round_loop(
  unsigned nThreads, int nRounds, GOSoundTaskTestImpl *pTask) {
  std::barrier roundBarrier(
    static_cast<std::ptrdiff_t>(nThreads), [pTask]() noexcept {
      if (pTask)
        pTask->NewRound();
    });
  std::vector<std::thread> threads;

  const auto start = std::chrono::high_resolution_clock::now();

  for (unsigned threadI = 0; threadI < nThreads; threadI++)
    threads.emplace_back([pTask, &roundBarrier, nRounds]() {
      for (int roundI = 0; roundI < nRounds; roundI++) {
        if (pTask)
          pTask->Run();
        roundBarrier.arrive_and_wait();
      }
    });
  for (std::thread &thread : threads)
    thread.join();

  const auto end = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = end - start;

  return elapsed.count();
}

void GOTestPerfSoundTaskBase::TestPerfUncontendedRunThroughput() {
  constexpr long N_ITERATIONS = 50'000'000;

  GOSoundTaskTestImpl task;

  task.Run(); // reaches RUN_STATE_DONE; DoRun() itself is not measured here

  const auto start = std::chrono::high_resolution_clock::now();

  for (long iterI = 0; iterI < N_ITERATIONS; iterI++)
    task.Run();

  const auto end = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = end - start;
  const double runsPerSecond = N_ITERATIONS / elapsed.count();
  const bool isPassed = runsPerSecond >= MIN_UNCONTENDED_RUNS_PER_SECOND;
  const double ratio = runsPerSecond / MIN_UNCONTENDED_RUNS_PER_SECOND;

  std::cout << std::format(
    "  [{}] UncontendedRunThroughput: {:.1f} runs/sec (threshold: {:.1f}, "
    "ratio: {:5.2f}x)\n",
    isPassed ? "PASS" : "FAIL",
    runsPerSecond,
    MIN_UNCONTENDED_RUNS_PER_SECOND,
    ratio);
  if (!isPassed)
    m_failedTests.push_back(std::format(
      "UncontendedRunThroughput: {:.1f} runs/sec < {:.1f}",
      runsPerSecond,
      MIN_UNCONTENDED_RUNS_PER_SECOND));
}

void GOTestPerfSoundTaskBase::TestPerfContendedRunLatency() {
  constexpr int N_ROUNDS = 2000;
  constexpr int WORK_MICROSECONDS = 10;

  for (const unsigned nThreads : {1u, 2u, 4u, 8u}) {
    GOSoundTaskTestImpl task;

    task.doRunWorkMicroseconds.store(WORK_MICROSECONDS);

    // Threads block on the barrier between rounds (no spinning), so nThreads
    // worker threads never oversubscribe the machine's cores by racing a
    // busy-wait alongside them. Thread start-up is included in the timing,
    // same as the generous margin on MAX_CONTENDED_ROUND_MILLISECONDS.
    std::barrier roundBarrier(
      static_cast<std::ptrdiff_t>(nThreads),
      [&task]() noexcept { task.NewRound(); });
    std::vector<std::thread> threads;

    const auto start = std::chrono::high_resolution_clock::now();

    for (unsigned threadI = 0; threadI < nThreads; threadI++)
      threads.emplace_back([&task, &roundBarrier]() {
        for (int roundI = 0; roundI < N_ROUNDS; roundI++) {
          task.Run();
          roundBarrier.arrive_and_wait();
        }
      });
    for (std::thread &thread : threads)
      thread.join();

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed = end - start;
    const double roundsPerSecond = N_ROUNDS / elapsed.count();
    const bool isPassed = roundsPerSecond >= MIN_CONTENDED_ROUNDS_PER_SECOND;
    const double ratio = roundsPerSecond / MIN_CONTENDED_ROUNDS_PER_SECOND;

    std::cout << std::format(
      "  [{}] ContendedRunLatency(nThreads={}): {:.1f} rounds/sec "
      "(threshold: {:.1f}, ratio: {:5.2f}x)\n",
      isPassed ? "PASS" : "FAIL",
      nThreads,
      roundsPerSecond,
      MIN_CONTENDED_ROUNDS_PER_SECOND,
      ratio);
    if (!isPassed)
      m_failedTests.push_back(std::format(
        "ContendedRunLatency(nThreads={}): {:.1f} rounds/sec < {:.1f}",
        nThreads,
        roundsPerSecond,
        MIN_CONTENDED_ROUNDS_PER_SECOND));
  }
}

void GOTestPerfSoundTaskBase::TestPerfCooperativeThroughput() {
  constexpr int N_ROUNDS = 1000;
  constexpr long N_WORK_ITEMS_PER_THREAD = 20000;

  for (const unsigned nThreads : {1u, 2u, 4u, 8u}) {
    GOSoundCooperativeTaskTestImpl task;

    // Per-thread quota rather than one shared pool drained item by item:
    // with a shared pool this loop measures cache-line ping-pong on that
    // one counter (20M contended atomics per run against ~16k lock
    // acquisitions), which is not what the round protocol costs.
    task.SetWorkItemsPerThread(N_WORK_ITEMS_PER_THREAD);

    // Same reusable thread pool + blocking barrier as
    // TestPerfContendedRunLatency, instead of spawning nThreads threads
    // per round: avoids both busy-wait oversubscription and per-round
    // thread creation overhead.
    std::barrier roundBarrier(
      static_cast<std::ptrdiff_t>(nThreads),
      [&task]() noexcept { task.NewRound(); });
    std::vector<std::thread> threads;

    const auto start = std::chrono::high_resolution_clock::now();

    for (unsigned threadI = 0; threadI < nThreads; threadI++)
      threads.emplace_back([&task, &roundBarrier]() {
        for (int roundI = 0; roundI < N_ROUNDS; roundI++) {
          task.Run();
          roundBarrier.arrive_and_wait();
        }
      });
    for (std::thread &thread : threads)
      thread.join();

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed = end - start;
    // Not N_WORK_ITEMS_PER_THREAD * nThreads * N_ROUNDS: a thread whose
    // Run() call arrives after the round is already RUN_STATE_DONE does no
    // work that round, which a fixed-quota assumption would still credit -
    // count what was actually processed instead
    const long nTotalItems = task.nTotalProcessedItems.load();
    const double itemsPerSecond = nTotalItems / elapsed.count();
    const bool isPassed = itemsPerSecond >= MIN_COOPERATIVE_ITEMS_PER_SECOND;
    const double ratio = itemsPerSecond / MIN_COOPERATIVE_ITEMS_PER_SECOND;

    std::cout << std::format(
      "  [{}] CooperativeThroughput(nThreads={}): {:.1f} items/sec "
      "(threshold: {:.1f}, ratio: {:5.2f}x)\n",
      isPassed ? "PASS" : "FAIL",
      nThreads,
      itemsPerSecond,
      MIN_COOPERATIVE_ITEMS_PER_SECOND,
      ratio);
    if (!isPassed)
      m_failedTests.push_back(std::format(
        "CooperativeThroughput(nThreads={}): {:.1f} items/sec < {:.1f}",
        nThreads,
        itemsPerSecond,
        MIN_COOPERATIVE_ITEMS_PER_SECOND));
  }
}

void GOTestPerfSoundTaskBase::TestPerfRoundProtocolCost() {
  constexpr int N_ROUNDS = 20000;

  for (const unsigned nThreads : {1u, 2u, 4u, 8u}) {
    GOSoundTaskTestImpl task;

    // DoRun() sleeps for nothing and does no work, so the only difference
    // between the two loops below is the round protocol itself
    const double withProtocolSeconds
      = measure_round_loop(nThreads, N_ROUNDS, &task);
    const double controlSeconds
      = measure_round_loop(nThreads, N_ROUNDS, nullptr);
    const double nanosecondsPerRound
      = (withProtocolSeconds - controlSeconds) * 1e9 / N_ROUNDS;
    const bool isPassed
      = nanosecondsPerRound <= MAX_PROTOCOL_NANOSECONDS_PER_ROUND;
    const double ratio
      = MAX_PROTOCOL_NANOSECONDS_PER_ROUND / nanosecondsPerRound;

    std::cout << std::format(
      "  [{}] RoundProtocolCost(nThreads={}): {:.1f} ns/round "
      "(threshold: {:.1f}, ratio: {:5.2f}x)\n",
      isPassed ? "PASS" : "FAIL",
      nThreads,
      nanosecondsPerRound,
      MAX_PROTOCOL_NANOSECONDS_PER_ROUND,
      ratio);
    if (!isPassed)
      m_failedTests.push_back(std::format(
        "RoundProtocolCost(nThreads={}): {:.1f} ns/round > {:.1f}",
        nThreads,
        nanosecondsPerRound,
        MAX_PROTOCOL_NANOSECONDS_PER_ROUND));
  }
}

void GOTestPerfSoundTaskBase::TestPerfNextPeriodLatency() {
  // 5000 periods at 32 frames / 96000 Hz is ~1.7 seconds of audio
  constexpr int N_PERIODS = 5000;
  // one task's share of a 333.33 us period, not the whole period
  constexpr int WORK_MICROSECONDS = 5;
  // workers retry well within a period, so contention stays realistic
  constexpr int WORKER_GAP_MICROSECONDS = 20;

  for (const unsigned nThreads : {1u, 2u, 4u, 8u}) {
    GOSoundTaskTestImpl task;
    std::atomic<bool> isStopped{false};
    std::vector<std::thread> workers;

    task.doRunWorkMicroseconds.store(WORK_MICROSECONDS);

    // The workers sleep between attempts rather than spinning: nThreads
    // spinning workers plus this thread would oversubscribe the machine and
    // turn the measurement into one of OS scheduling instead of contention.
    for (unsigned threadI = 0; threadI < nThreads; threadI++)
      workers.emplace_back([&task, &isStopped, WORKER_GAP_MICROSECONDS]() {
        while (!isStopped.load()) {
          task.Run();
          std::this_thread::sleep_for(
            std::chrono::microseconds(WORKER_GAP_MICROSECONDS));
        }
      });

    double totalMicroseconds = 0;
    double maxMicroseconds = 0;

    for (int periodI = 0; periodI < N_PERIODS; periodI++) {
      const auto start = std::chrono::high_resolution_clock::now();

      // exactly what GOSoundOrganEngine::NextPeriod() does per audio period
      task.CompleteRound();
      task.NewRound();

      const auto end = std::chrono::high_resolution_clock::now();
      const double microseconds
        = std::chrono::duration<double, std::micro>(end - start).count();

      totalMicroseconds += microseconds;
      if (microseconds > maxMicroseconds)
        maxMicroseconds = microseconds;
    }
    isStopped.store(true);
    for (std::thread &worker : workers)
      worker.join();

    const double meanMicroseconds = totalMicroseconds / N_PERIODS;
    const bool isPassed = meanMicroseconds <= MAX_NEXT_PERIOD_MEAN_MICROSECONDS
      && maxMicroseconds <= MAX_NEXT_PERIOD_PEAK_MICROSECONDS;
    // how much of one audio period the handshake alone consumes
    const double meanPeriodPercent
      = 100.0 * meanMicroseconds / AUDIO_PERIOD_MICROSECONDS;
    const double maxPeriods = maxMicroseconds / AUDIO_PERIOD_MICROSECONDS;

    std::cout << std::format(
      "  [{}] NextPeriodLatency(nThreads={}): mean {:.1f} us ({:.1f}% of a "
      "{:.1f} us period, threshold {:.1f}), max {:.1f} us ({:.1f} periods, "
      "threshold {:.1f})\n",
      isPassed ? "PASS" : "FAIL",
      nThreads,
      meanMicroseconds,
      meanPeriodPercent,
      AUDIO_PERIOD_MICROSECONDS,
      MAX_NEXT_PERIOD_MEAN_MICROSECONDS,
      maxMicroseconds,
      maxPeriods,
      MAX_NEXT_PERIOD_PEAK_MICROSECONDS);
    if (!isPassed)
      m_failedTests.push_back(std::format(
        "NextPeriodLatency(nThreads={}): mean {:.1f} us (max allowed {:.1f}), "
        "max {:.1f} us (max allowed {:.1f})",
        nThreads,
        meanMicroseconds,
        MAX_NEXT_PERIOD_MEAN_MICROSECONDS,
        maxMicroseconds,
        MAX_NEXT_PERIOD_PEAK_MICROSECONDS));
  }
}

void GOTestPerfSoundTaskBase::TestPerfLazyPrerequisiteAccess() {
  constexpr int N_ROUNDS = 5000;
  constexpr int WORK_MICROSECONDS = 5;

  for (const bool isDoneWithinRound : {true, false}) {
    for (const unsigned nThreads : {1u, 2u, 4u, 8u}) {
      GOSoundTaskTestImpl task;
      std::atomic<int> nStalePayloads{0};

      task.doRunResult.store(isDoneWithinRound);
      task.doRunWorkMicroseconds.store(WORK_MICROSECONDS);

      std::barrier roundBarrier(
        static_cast<std::ptrdiff_t>(nThreads),
        [&task]() noexcept { task.NewRound(); });
      std::vector<std::thread> threads;

      const auto start = std::chrono::high_resolution_clock::now();

      for (unsigned threadI = 0; threadI < nThreads; threadI++)
        threads.emplace_back([&task, &roundBarrier, &nStalePayloads]() {
          for (int roundI = 0; roundI < N_ROUNDS; roundI++) {
            // DoRun() publishes a strictly positive payload, so a zero here
            // means this thread returned from Run() before the round's
            // result was ready
            if (task.GetPayloadLazily() == 0)
              nStalePayloads.fetch_add(1);
            roundBarrier.arrive_and_wait();
          }
        });
      for (std::thread &thread : threads)
        thread.join();

      const auto end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsed = end - start;
      const double roundsPerSecond = N_ROUNDS / elapsed.count();
      const int nStale = nStalePayloads.load();
      const bool isPassed
        = roundsPerSecond >= MIN_LAZY_ROUNDS_PER_SECOND && nStale == 0;
      const double ratio = roundsPerSecond / MIN_LAZY_ROUNDS_PER_SECOND;

      std::cout << std::format(
        "  [{}] LazyPrerequisiteAccess(doneWithinRound={}, nThreads={}): "
        "{:.1f} rounds/sec (threshold: {:.1f}, ratio: {:5.2f}x), "
        "stale payloads: {}\n",
        isPassed ? "PASS" : "FAIL",
        isDoneWithinRound,
        nThreads,
        roundsPerSecond,
        MIN_LAZY_ROUNDS_PER_SECOND,
        ratio,
        nStale);
      if (!isPassed)
        m_failedTests.push_back(std::format(
          "LazyPrerequisiteAccess(doneWithinRound={}, nThreads={}): "
          "{:.1f} rounds/sec < {:.1f}, stale payloads: {}",
          isDoneWithinRound,
          nThreads,
          roundsPerSecond,
          MIN_LAZY_ROUNDS_PER_SECOND,
          nStale));
    }
  }
}

void GOTestPerfSoundTaskBase::run() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundTaskBase "
               "==========\n";

  TestPerfUncontendedRunThroughput();
  TestPerfContendedRunLatency();
  TestPerfCooperativeThroughput();
  TestPerfRoundProtocolCost();
  TestPerfNextPeriodLatency();
  TestPerfLazyPrerequisiteAccess();

  std::cout << "\n========== Performance Tests Completed ==========\n";

  if (!m_failedTests.empty()) {
    std::string errorMsg
      = std::format("{} performance test(s) failed:\n", m_failedTests.size());

    for (const auto &failedTest : m_failedTests)
      errorMsg += "  - " + failedTest + "\n";
    GOAssert(false, errorMsg);
  }
}
