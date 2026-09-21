/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundCallbackConnector.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <thread>

#include "sound/GOSoundOrganEngine.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/interfaces/GOSoundCallbackConnector.h"
#include "sound/tasks/GOSoundRecorderTask.h"

#include "GOTestScope.h"

const std::string GOTestSoundCallbackConnector::TEST_NAME
  = "GOTestSoundCallbackConnector";

static struct TestConnector : public GOSoundCallbackConnector {
  std::atomic_uint m_NConnects{0};
  std::atomic_uint m_NDisconnects{0};
  std::atomic_uint m_NNewPeriods{0};

  void OnBeforeConnectToEngine() override { m_NConnects.fetch_add(1); }
  void OnAfterDisconnectFromEngine() override { m_NDisconnects.fetch_add(1); }
  void OnNewAudioPeriod() override { m_NNewPeriods.fetch_add(1); }

  void Setup(unsigned nSamplesPerBuffer, unsigned sampleRate) {
    m_NConnects.store(0);
    m_NDisconnects.store(0);
    m_NNewPeriods.store(0);
    SetSamplesPerBuffer(nSamplesPerBuffer);
    SetSampleRate(sampleRate);
  }
} testConnector;

void GOTestSoundCallbackConnector::TestSilenceWithoutEngine() {
  std::atomic_bool isRunning{true};

  auto threadBody = [&]() {
    for (unsigned iterI = 0; iterI < 100; ++iterI) {
      GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
        buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

      m_connector.AudioCallback(0, buf);

      const float *pData = buf.GetData();
      const unsigned nItems = buf.GetNItems();
      bool isSilent = true;

      for (unsigned itemI = 0; itemI < nItems; ++itemI) {
        if (pData[itemI] != 0.0f) {
          isSilent = false;
          break;
        }
      }

      GOAssert(isSilent, "AudioCallback without engine should produce silence");
    }
  };

  std::thread t1(threadBody);
  std::thread t2(threadBody);
  std::thread t3(threadBody);
  std::thread t4(threadBody);

  t1.join();
  t2.join();
  t3.join();
  t4.join();
}

void GOTestSoundCallbackConnector::TestConnectDisconnectLifecycle() {
  testConnector.Setup(N_SAMPLES_PER_BUFFER, SAMPLE_RATE);

  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  for (unsigned cycleI = 0; cycleI < 2; ++cycleI) {
    testConnector.ConnectToEngine(engine);

    GOAssert(
      testConnector.m_NConnects.load() == cycleI + 1,
      "OnBeforeConnectToEngine should be called once per connect");

    testConnector.DisconnectFromEngine(engine);

    GOAssert(
      testConnector.m_NDisconnects.load() == cycleI + 1,
      "OnAfterDisconnectFromEngine should be called once per disconnect");
  }

  StopAndDestroyEngine();
}

void GOTestSoundCallbackConnector::TestAsyncCallbacksXrun() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 1);

  m_connector.ConnectToEngine(engine);

  std::atomic_bool isRunning{true};

  auto threadBody = [&]() {
    while (isRunning.load()) {
      GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
        buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

      m_connector.AudioCallback(0, buf);
    }
  };

  std::thread t1(threadBody);
  std::thread t2(threadBody);
  std::thread t3(threadBody);

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  m_connector.DisconnectFromEngine(engine);

  isRunning.store(false);
  t1.join();
  t2.join();
  t3.join();

  StopAndDestroyEngine();
}

void GOTestSoundCallbackConnector::TestConnectDisconnectCyclesAsyncCallbacks() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  std::atomic_bool isRunning{true};

  // Drives both of the engine's 2 outputs (not the same one twice): only
  // then can a period boundary ever be reached, which is what lets most of
  // this test's disconnects go through EnsureStreamingDisableAllowed()'s
  // graceful path instead of its ~1s timeout fallback.
  auto threadBody = [&](unsigned outputIndex) {
    while (isRunning.load()) {
      GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
        buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

      m_connector.AudioCallback(outputIndex, buf);
    }
  };

  std::thread t1(threadBody, 0);
  std::thread t2(threadBody, 1);

  for (unsigned cycleI = 0; cycleI < 30; ++cycleI) {
    m_connector.ConnectToEngine(engine);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    m_connector.DisconnectFromEngine(engine);
  }

  isRunning.store(false);
  t1.join();
  t2.join();

  StopAndDestroyEngine();
}

void GOTestSoundCallbackConnector::TestDisconnectWaitsAsyncCallbacks() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  m_connector.ConnectToEngine(engine);

  std::atomic_bool isRunning{true};

  auto threadBody = [&]() {
    while (isRunning.load()) {
      GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
        buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

      m_connector.AudioCallback(0, buf);
    }
  };

  std::thread t1(threadBody);
  std::thread t2(threadBody);
  std::thread t3(threadBody);
  std::thread t4(threadBody);

  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  m_connector.DisconnectFromEngine(engine);

  isRunning.store(false);
  t1.join();
  t2.join();
  t3.join();
  t4.join();

  StopAndDestroyEngine();
}

void GOTestSoundCallbackConnector::TestGracefulDisconnectDoesNotCutPeriod() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  m_connector.ConnectToEngine(engine);

  const uint64_t timeBeforePeriod = engine.GetTime();

  // Unlike GOSoundOrganEngine::ProcessAudioCallback(),
  // GOSoundCallbackConnector::AudioCallback() always returns true (it is the
  // audio-port "keep the stream open" signal, not a period-advance flag), so
  // the period boundary is observed via engine.GetTime() below instead.
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(0, buf0);

  GOAssert(
    engine.GetTime() == timeBeforePeriod,
    "output 0 of 2 alone should not advance the period");

  std::atomic_bool isDisconnectStarted{false};
  const auto disconnectStartTime = std::chrono::steady_clock::now();
  std::chrono::steady_clock::duration disconnectDuration{};

  std::thread disconnectThread([&]() {
    isDisconnectStarted.store(true);
    m_connector.DisconnectFromEngine(engine);
    disconnectDuration = std::chrono::steady_clock::now() - disconnectStartTime;
  });

  while (!isDisconnectStarted.load())
    std::this_thread::yield();
  // Give EnsureStreamingDisableAllowed() time to publish the request flag
  // and enqueue on the condition before output 1 is driven below.
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  // Output 1 completes the same period the graceful branch is waiting for -
  // it must still get its real buffer, not the !IsStreaming() silence path.
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(1, buf1);

  disconnectThread.join();

  GOAssert(
    disconnectDuration < std::chrono::milliseconds(500),
    "a graceful disconnect with callbacks still arriving should not wait "
    "anywhere near the 1s timeout");
  GOAssert(
    engine.GetTime() == timeBeforePeriod + N_SAMPLES_PER_BUFFER,
    "the period is finished by FinishPeriod() as soon as the graceful "
    "boundary is reached, inside the racing callback itself - not deferred "
    "until StopEngine()");

  engine.StopEngine();

  GOAssert(
    engine.GetTime() == timeBeforePeriod + N_SAMPLES_PER_BUFFER,
    "StopEngine() does not advance the clock further: the period was "
    "already finished above, and StopEngine() only ever renews counters, "
    "never accounts a period itself");

  engine.DestroyEngine();
  GOAssert(engine.IsIdle(), "Engine should be IDLE after DestroyEngine");
}

void GOTestSoundCallbackConnector::
  TestGracefulStopFlushesLastPeriodToRecorder() {
  const unsigned nOutputs = 2;
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ nOutputs);
  GOSoundRecorderTask &recorder = engine.GetRecorderTask();
  const std::filesystem::path path = std::filesystem::temp_directory_path()
    / ("GOTestRecorder-"
       + std::to_string(
         std::chrono::steady_clock::now().time_since_epoch().count())
       + ".wav");

  // Baseline: header-only file size, with no periods ever recorded.
  recorder.Open(path.string());
  recorder.Close();

  const uint64_t emptySize = std::filesystem::file_size(path);

  recorder.Open(path.string());
  m_connector.ConnectToEngine(engine);

  const unsigned nOrdinaryPeriods = 2;

  for (unsigned periodI = 0; periodI < nOrdinaryPeriods; ++periodI) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    m_connector.AudioCallback(0, buf0);
    m_connector.AudioCallback(1, buf1);
  }

  // Race a graceful DisconnectFromEngine() against output 1 finishing the
  // period - same technique as TestGracefulDisconnectDoesNotCutPeriod, needed
  // so the disconnect lands exactly on the period boundary instead of the
  // ~1s timeout. With 0 aux threads, the recorder task can only ever run via
  // CompleteRound(), so this is the deterministic case for the bug this test
  // targets: StopEngine() finalizing this last period without running it.
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf0g, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(0, buf0g);

  std::atomic_bool isDisconnectStarted{false};

  std::thread disconnectThread([&]() {
    isDisconnectStarted.store(true);
    m_connector.DisconnectFromEngine(engine);
  });

  while (!isDisconnectStarted.load())
    std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf1g, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(1, buf1g);

  disconnectThread.join();

  engine.StopEngine();
  recorder.Close();

  const unsigned nTotalPeriods = nOrdinaryPeriods + 1;
  const unsigned periodBytes = N_SAMPLES_PER_BUFFER
    * (nOutputs * N_OUTPUT_CHANNELS) * recorder.GetBytesPerSample();
  const uint64_t expectedSize = emptySize + nTotalPeriods * periodBytes;

  GOAssert(
    std::filesystem::file_size(path) == expectedSize,
    "the graceful stop's last period must be flushed to the recorder by "
    "StopEngine(), not dropped when StartEngine()'s NewRound() later resets "
    "the round without ever having run it");

  std::filesystem::remove(path);
  engine.DestroyEngine();
  GOAssert(engine.IsIdle(), "Engine should be IDLE after DestroyEngine");
}

void GOTestSoundCallbackConnector::TestDisconnectTimesOutWithoutCallbacks() {
  // The engine is STREAMING but no audio callback ever arrives, so
  // EnsureStreamingDisableAllowed() cannot see a period boundary and must
  // fall back to its ~1s timeout.
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 1);

  m_connector.ConnectToEngine(engine);

  const uint64_t timeBeforeStop = engine.GetTime();
  const auto start = std::chrono::steady_clock::now();

  m_connector.DisconnectFromEngine(engine);

  const auto elapsed = std::chrono::steady_clock::now() - start;

  GOAssert(
    elapsed >= std::chrono::milliseconds(900),
    "disconnecting a streaming engine with no callbacks should take "
    "roughly the full timeout, not return early");

  engine.StopEngine();

  GOAssert(
    engine.GetTime() == timeBeforeStop,
    "StopEngine() must not advance the clock after a timeout fallback: no "
    "period was ever delivered to every output, so nothing was actually "
    "played");

  engine.DestroyEngine();
  GOAssert(engine.IsIdle(), "Engine should be IDLE after DestroyEngine");
}

void GOTestSoundCallbackConnector::TestStopEngineWithoutStreaming() {
  // Never connected and never streamed: StopEngine() has nothing to
  // finalize.
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 1);
  const uint64_t timeBeforeStop = engine.GetTime();

  engine.StopEngine();

  GOAssert(
    engine.GetTime() == timeBeforeStop,
    "StopEngine() must not advance the clock when the engine was never "
    "connected");

  engine.DestroyEngine();
  GOAssert(engine.IsIdle(), "Engine should be IDLE after DestroyEngine");
}

void GOTestSoundCallbackConnector::TestBareReconnectAfterGracefulDisconnect() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  m_connector.ConnectToEngine(engine);

  const uint64_t timeAtStart = engine.GetTime();

  // Race a graceful DisconnectFromEngine() against output 1 finishing the
  // period - same technique as TestGracefulDisconnectDoesNotCutPeriod, needed
  // so the disconnect lands exactly on the period boundary instead of the
  // ~1s timeout.
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(0, buf0);

  std::atomic_bool isDisconnectStarted{false};

  std::thread disconnectThread([&]() {
    isDisconnectStarted.store(true);
    m_connector.DisconnectFromEngine(engine);
  });

  while (!isDisconnectStarted.load())
    std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(1, buf1);

  disconnectThread.join();

  GOAssert(
    engine.GetTime() == timeAtStart + N_SAMPLES_PER_BUFFER,
    "the period is finished by FinishPeriod() as soon as the graceful "
    "boundary is reached, inside the racing callback itself");

  const uint64_t roundBeforeReconnect
    = engine.GetScheduler().GetRoundCounter().GetRoundNumber();

  // Bare reconnect: no StopEngine()/StartEngine() in between. The round left
  // pending by the graceful stop is started inside ConnectToEngine()
  // (SetStreaming(true) -> StartPeriod()), before any callback of the new
  // session runs - not by the first callback itself, and it does not
  // account the period again (that already happened above).
  m_connector.ConnectToEngine(engine);

  GOAssert(
    engine.GetScheduler().GetRoundCounter().GetRoundNumber()
      > roundBeforeReconnect,
    "reconnecting must open a fresh round instead of leaving the previous "
    "session's already-DONE one in place");
  GOAssert(
    engine.GetTime() == timeAtStart + N_SAMPLES_PER_BUFFER,
    "reconnecting must not account the graceful period a second time - it "
    "was already finished before the reconnect, above");

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf0b, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(0, buf0b);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf1b, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(1, buf1b);

  GOAssert(
    engine.GetTime() == timeAtStart + 2 * N_SAMPLES_PER_BUFFER,
    "the new session's own period should finish and be accounted normally");

  m_connector.DisconnectFromEngine(engine);
  StopAndDestroyEngine();
}

void GOTestSoundCallbackConnector::TestBareReconnectAfterTimeout() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  m_connector.ConnectToEngine(engine);

  const uint64_t timeAtStart = engine.GetTime();

  // Drive only output 0; output 1 never arrives, so
  // EnsureStreamingDisableAllowed() falls back to its ~1s timeout without a
  // graceful boundary - m_IsToStartPeriod stays unset.
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(0, buf0);
  m_connector.DisconnectFromEngine(engine);

  GOAssert(
    engine.GetTime() == timeAtStart,
    "an aborted period that never reached every output must never be "
    "accounted");

  // Bare reconnect: no StopEngine()/StartEngine() in between. A reconnect
  // leaves the interrupted period's state untouched, so it simply plays out
  // to completion instead of being discarded - only output 1, the one that
  // never arrived, is needed to finish it.
  m_connector.ConnectToEngine(engine);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(1, buf1);

  GOAssert(
    engine.GetTime() == timeAtStart + N_SAMPLES_PER_BUFFER,
    "output 1 alone completes the period output 0 already delivered before "
    "the timeout");

  uint64_t previousTime = engine.GetTime();

  for (unsigned periodI = 0; periodI < 3; ++periodI) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buf0i, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buf1i, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    m_connector.AudioCallback(0, buf0i);
    m_connector.AudioCallback(1, buf1i);

    GOAssert(
      engine.GetTime() == previousTime + N_SAMPLES_PER_BUFFER,
      "every period after the recovered one must advance the clock by "
      "exactly one buffer");
    previousTime = engine.GetTime();
  }

  m_connector.DisconnectFromEngine(engine);
  StopAndDestroyEngine();
}

void GOTestSoundCallbackConnector::TestSuspendResumeCycle() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 1, /* nAuxThreads */ 0, /* nOutputs */ 2);

  uint64_t expectedTime = engine.GetTime();

  for (unsigned cycleI = 0; cycleI < 3; ++cycleI) {
    m_connector.ConnectToEngine(engine);

    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

    m_connector.AudioCallback(0, buf0);
    m_connector.AudioCallback(1, buf1);

    expectedTime += N_SAMPLES_PER_BUFFER;
    GOAssert(
      engine.GetTime() == expectedTime,
      "a full driven period should advance the clock by exactly one buffer");

    m_connector.DisconnectFromEngine(engine);
    engine.StopEngine();
    engine.StartEngine();
  }

  // One suspend/resume cycle over an incomplete period (timeout fallback):
  // the clock must not advance for it, and the round left behind must not
  // survive the resume - that guarantee comes from StopEngine() abandoning
  // the interrupted period and StartEngine()/SetStreaming(true) opening a
  // fresh one, not from this fix, so this is a regression guard for the
  // already-supported Suspend/Resume path rather than new coverage.
  m_connector.ConnectToEngine(engine);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf0Partial, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(0, buf0Partial); // output 1 never arrives

  m_connector.DisconnectFromEngine(engine); // falls through the ~1s timeout
  engine.StopEngine();

  GOAssert(
    engine.GetTime() == expectedTime,
    "an incomplete period must not be accounted across a suspend");

  engine.StartEngine();
  m_connector.ConnectToEngine(engine);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf0R, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
    buf1R, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(0, buf0R);
  m_connector.AudioCallback(1, buf1R);

  expectedTime += N_SAMPLES_PER_BUFFER;
  GOAssert(
    engine.GetTime() == expectedTime,
    "the period after resume should compute fresh audio and advance the "
    "clock by exactly one buffer");

  m_connector.DisconnectFromEngine(engine);
  StopAndDestroyEngine();
}

void GOTestSoundCallbackConnector::run() {
  GO_RUN_TEST(TestSilenceWithoutEngine())
  GO_RUN_TEST(TestConnectDisconnectLifecycle())
  GO_RUN_TEST(TestAsyncCallbacksXrun())
  GO_RUN_TEST(TestConnectDisconnectCyclesAsyncCallbacks())
  GO_RUN_TEST(TestDisconnectWaitsAsyncCallbacks())
  GO_RUN_TEST(TestGracefulDisconnectDoesNotCutPeriod())
  GO_RUN_TEST(TestGracefulStopFlushesLastPeriodToRecorder())
  GO_RUN_TEST(TestDisconnectTimesOutWithoutCallbacks())
  GO_RUN_TEST(TestStopEngineWithoutStreaming())
  GO_RUN_TEST(TestBareReconnectAfterGracefulDisconnect())
  GO_RUN_TEST(TestBareReconnectAfterTimeout())
  GO_RUN_TEST(TestSuspendResumeCycle())
}
