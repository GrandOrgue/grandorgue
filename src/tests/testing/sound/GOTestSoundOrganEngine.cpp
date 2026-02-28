/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundOrganEngine.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <format>
#include <iterator>
#include <mutex>
#include <thread>

#include "sound/GOSoundOrganEngine.h"
#include "sound/buffer/GOSoundBufferMutable.h"

const std::string GOTestSoundOrganEngine::TEST_NAME = "GOTestSoundOrganEngine";

struct EngineConfig {
  unsigned nAudioGroups;
  unsigned nAuxThreads;
  unsigned nOutputs;
};

static const EngineConfig MULTIPLE_CONFIGS[] = {
  {1, 0, 1},
  {2, 0, 1},
  {1, 1, 1},
  {2, 1, 1},
  {1, 0, 2},
  {2, 0, 2},
  {1, 1, 2},
  {2, 1, 2},
};

static constexpr unsigned N_MULTIPLE_CONFIGS = std::size(MULTIPLE_CONFIGS);

GOSoundOrganEngine &GOTestSoundOrganEngine::BuildStartAndConnectEngine(
  unsigned nAudioGroups, unsigned nAuxThreads, unsigned nOutputs) {
  GOSoundOrganEngine &engine
    = BuildAndStartEngine(nAudioGroups, nAuxThreads, nOutputs);

  engine.SetUsed(true);
  engine.SetStreaming(true);

  GOAssert(
    engine.IsUsed() && engine.IsStreaming(),
    "Engine should be USED and STREAMING after connect");

  return engine;
}

void GOTestSoundOrganEngine::DisconnectStopAndDestroyEngine() {
  GOSoundOrganEngine &engine = controller->GetSoundEngine();

  engine.SetStreaming(false);
  engine.SetUsed(false);

  GOAssert(
    engine.IsWorking() && !engine.IsUsed() && !engine.IsStreaming(),
    "Engine should be WORKING and not USED after disconnect");

  StopAndDestroyEngine();
}

void GOTestSoundOrganEngine::TestSingleOutputLifecycle() {
  GOSoundOrganEngine &engine = BuildStartAndConnectEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 1);

  for (unsigned periodI = 0; periodI < 5; ++periodI) {
    GO_DECLARE_LOCAL_SOUND_BUFFER(buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    const bool didAdvance = engine.ProcessAudioCallback(0, buf);

    GOAssert(
      didAdvance,
      std::format(
        "Period {}: single output should advance the period", periodI));
  }

  DisconnectStopAndDestroyEngine();
}

void GOTestSoundOrganEngine::TestTwoOutputsLifecycleWith(
  unsigned nAudioGroups) {
  GOSoundOrganEngine &engine = BuildStartAndConnectEngine(nAudioGroups, 0, 2);

  for (unsigned periodI = 0; periodI < 5; ++periodI) {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    const bool didAdvanceAfter0 = engine.ProcessAudioCallback(0, buf0);
    const bool didAdvanceAfter1 = engine.ProcessAudioCallback(1, buf1);

    GOAssert(
      !didAdvanceAfter0,
      std::format(
        "Period {}: output 0 of 2 should not advance the period", periodI));
    GOAssert(
      didAdvanceAfter1,
      std::format(
        "Period {}: output 1 of 2 should advance the period", periodI));
  }

  DisconnectStopAndDestroyEngine();
}

void GOTestSoundOrganEngine::TestTwoOutputsLifecycle() {
  TestTwoOutputsLifecycleWith(1);
}

void GOTestSoundOrganEngine::TestTwoGroupsTwoOutputsLifecycle() {
  TestTwoOutputsLifecycleWith(2);
}

void GOTestSoundOrganEngine::TestSetUsedTransitions() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 1);

  for (unsigned cycleI = 0; cycleI < 3; ++cycleI) {
    GOAssert(
      engine.IsWorking() && !engine.IsUsed() && !engine.IsStreaming(),
      std::format(
        "Cycle {}: engine should be WORKING before SetUsed(true)", cycleI));

    engine.SetUsed(true);

    GOAssert(
      engine.IsUsed() && !engine.IsStreaming(),
      std::format(
        "Cycle {}: engine should be USED after SetUsed(true)", cycleI));

    engine.SetStreaming(true);

    GOAssert(
      engine.IsUsed() && engine.IsStreaming(),
      std::format(
        "Cycle {}: engine should be STREAMING after SetStreaming(true)",
        cycleI));

    engine.SetStreaming(false);

    GOAssert(
      engine.IsUsed() && !engine.IsStreaming(),
      std::format(
        "Cycle {}: engine should be USED after SetStreaming(false)", cycleI));

    engine.SetUsed(false);

    GOAssert(
      engine.IsWorking() && !engine.IsUsed() && !engine.IsStreaming(),
      std::format(
        "Cycle {}: engine should be WORKING after SetUsed(false)", cycleI));
  }

  StopAndDestroyEngine();
}

void GOTestSoundOrganEngine::TestBuildStopCyclesAsyncCallbacksXrun() {
  for (unsigned cycleI = 0; cycleI < 100; ++cycleI) {
    GOSoundOrganEngine &engine = BuildStartAndConnectEngine(
      /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 1);

    std::atomic_bool isRunning{true};

    auto threadBody = [&]() {
      while (isRunning.load()) {
        GO_DECLARE_LOCAL_SOUND_BUFFER(
          buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

        engine.ProcessAudioCallback(0, buf);
      }
    };

    std::thread t1(threadBody);
    std::thread t2(threadBody);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    isRunning.store(false);
    t1.join();
    t2.join();

    DisconnectStopAndDestroyEngine();
  }
}

void GOTestSoundOrganEngine::TestMultipleConfigsAsyncCallbacks() {
  static constexpr unsigned N_PERIODS = 100;

  for (unsigned cycleI = 0; cycleI < 100; ++cycleI) {
    const EngineConfig &cfg = MULTIPLE_CONFIGS[cycleI % N_MULTIPLE_CONFIGS];

    GOSoundOrganEngine &engine = BuildStartAndConnectEngine(
      cfg.nAudioGroups, cfg.nAuxThreads, cfg.nOutputs);

    std::vector<std::thread> threads;

    for (unsigned outputI = 0; outputI < cfg.nOutputs; ++outputI) {
      threads.emplace_back([&, outputI]() {
        for (unsigned periodI = 0; periodI < N_PERIODS; ++periodI) {
          GO_DECLARE_LOCAL_SOUND_BUFFER(
            buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

          engine.ProcessAudioCallback(outputI, buf);
        }
      });
    }

    for (std::thread &t : threads)
      t.join();

    DisconnectStopAndDestroyEngine();
  }
}

void GOTestSoundOrganEngine::TestDisconnectWithXrunDeadlock() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  engine.SetUsed(true);
  engine.SetStreaming(true);

  // Period 0: complete normally so the engine is ready for period 1.
  {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    engine.ProcessAudioCallback(0, buf0);
    engine.ProcessAudioCallback(1, buf1);
  }

  // Period 1, output 0 (first call) — marks state.wait=true for output 0.
  // Output 1 is not yet processed, so the period has not advanced.
  {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    engine.ProcessAudioCallback(0, buf0);
  }

  // Simulate GOSoundSystem's m_NCallbacksEntered / disconnect logic.
  std::atomic_uint nActiveCallbacks{0};
  std::mutex mu;
  std::condition_variable cv;
  std::atomic_bool isStopping{false};
  std::atomic_bool isRunning{true};

  // Audio thread: calls PAC(0) in a tight loop (xrun on output 0).
  // Since state.wait=true and IsStreaming()=true, the first call immediately
  // blocks at [W1].
  std::thread audioThread([&]() {
    while (isRunning.load()) {
      if (!isStopping.load()) {
        ++nActiveCallbacks;

        GO_DECLARE_LOCAL_SOUND_BUFFER(
          buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

        engine.ProcessAudioCallback(0, buf); // blocks at [W1]

        if (--nActiveCallbacks == 0 && isStopping.load())
          cv.notify_all();
      }
    }
  });

  // Spin until the audio thread is inside PAC (blocked at [W1]).
  while (nActiveCallbacks.load() == 0)
    std::this_thread::yield();

  // Mirrors GOSoundSystem::DisconnectFromEngine:
  // stop accepting new callbacks, then unblock callbacks waiting at [W1].
  isStopping.store(true);
  engine.SetStreaming(false);

  // SetStreaming(false) broadcast [W1]: PAC(0) exits early (fills silence),
  // nActiveCallbacks drops to 0, and the wait completes within 1s.
  bool didFinish;

  {
    std::unique_lock<std::mutex> lk(mu);

    didFinish = cv.wait_for(lk, std::chrono::seconds(1), [&] {
      return nActiveCallbacks.load() == 0;
    });
  }

  // Audio thread has exited PAC; let it see isRunning=false and terminate.
  isRunning.store(false);
  audioThread.join();

  engine.SetUsed(false);
  StopAndDestroyEngine();

  GOAssert(
    didFinish,
    "DisconnectFromEngine should not deadlock: all active callbacks should "
    "finish within 1s of isStopping being set");
}

void GOTestSoundOrganEngine::TestReconnectAfterMidPeriodDisconnect() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 3);

  // First streaming session: process only outputs 0 and 1, leaving the period
  // incomplete. This leaves m_NCallbacksEntered=2 and m_NCallbacksFinished=2.
  engine.SetUsed(true);
  engine.SetStreaming(true);

  {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    engine.ProcessAudioCallback(0, buf0);
    engine.ProcessAudioCallback(1, buf1);
  }

  engine.SetStreaming(false);
  engine.SetUsed(false);

  // Second streaming session: SetStreaming(true) must reset the dirty counters.
  // Without the reset, output 0 alone would advance the period (dirty counter
  // makes nCallbacksFinished reach nOutputs after just one call), leaving
  // outputs 1 and 2 blocked at [W1] with wasProcessedInCurrentPeriod=true.
  engine.SetUsed(true);
  engine.SetStreaming(true);

  for (unsigned periodI = 0; periodI < 5; ++periodI) {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      buf2, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    const bool didAdvanceAfter0 = engine.ProcessAudioCallback(0, buf0);
    const bool didAdvanceAfter1 = engine.ProcessAudioCallback(1, buf1);
    const bool didAdvanceAfter2 = engine.ProcessAudioCallback(2, buf2);

    GOAssert(
      !didAdvanceAfter0 && !didAdvanceAfter1 && didAdvanceAfter2,
      std::format(
        "Period {}: only output 2 should advance the period", periodI));
  }

  engine.SetStreaming(false);
  engine.SetUsed(false);
  StopAndDestroyEngine();
}

void GOTestSoundOrganEngine::run() {
  TestSingleOutputLifecycle();
  TestTwoOutputsLifecycle();
  TestTwoGroupsTwoOutputsLifecycle();
  TestSetUsedTransitions();
  TestBuildStopCyclesAsyncCallbacksXrun();
  TestMultipleConfigsAsyncCallbacks();
  TestDisconnectWithXrunDeadlock();
  TestReconnectAfterMidPeriodDisconnect();
}
