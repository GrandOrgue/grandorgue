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
#include <thread>

#include "sound/GOSoundOrganEngine.h"
#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/interfaces/GOSoundCallbackConnector.h"

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
      GO_DECLARE_LOCAL_SOUND_BUFFER(
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
      GO_DECLARE_LOCAL_SOUND_BUFFER(
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

  auto threadBody = [&]() {
    while (isRunning.load()) {
      GO_DECLARE_LOCAL_SOUND_BUFFER(
        buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

      m_connector.AudioCallback(0, buf);
    }
  };

  std::thread t1(threadBody);
  std::thread t2(threadBody);

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
      GO_DECLARE_LOCAL_SOUND_BUFFER(
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
  GO_DECLARE_LOCAL_SOUND_BUFFER(buf0, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

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
  GO_DECLARE_LOCAL_SOUND_BUFFER(buf1, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

  m_connector.AudioCallback(1, buf1);

  disconnectThread.join();

  GOAssert(
    disconnectDuration < std::chrono::milliseconds(500),
    "a graceful disconnect with callbacks still arriving should not wait "
    "anywhere near the 1s timeout");
  GOAssert(
    engine.GetTime() == timeBeforePeriod,
    "the clock should not advance before StopEngine()");

  engine.StopEngine();

  GOAssert(
    engine.GetTime() == timeBeforePeriod + N_SAMPLES_PER_BUFFER,
    "StopEngine() should advance the clock by exactly the one period the "
    "graceful branch signalled as complete");

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
    engine.GetTime() == timeBeforeStop + N_SAMPLES_PER_BUFFER,
    "StopEngine() should still advance the clock by one period after a "
    "timeout fallback");

  engine.DestroyEngine();
  GOAssert(engine.IsIdle(), "Engine should be IDLE after DestroyEngine");
}

void GOTestSoundCallbackConnector::run() {
  GO_RUN_TEST(TestSilenceWithoutEngine())
  GO_RUN_TEST(TestConnectDisconnectLifecycle())
  GO_RUN_TEST(TestAsyncCallbacksXrun())
  GO_RUN_TEST(TestConnectDisconnectCyclesAsyncCallbacks())
  GO_RUN_TEST(TestDisconnectWaitsAsyncCallbacks())
  GO_RUN_TEST(TestGracefulDisconnectDoesNotCutPeriod())
  GO_RUN_TEST(TestDisconnectTimesOutWithoutCallbacks())
}
