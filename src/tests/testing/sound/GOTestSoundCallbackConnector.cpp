/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundCallbackConnector.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "sound/GOSoundOrganEngine.h"
#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/interfaces/GOSoundCallbackConnector.h"

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

void GOTestSoundCallbackConnector::run() {
  TestSilenceWithoutEngine();
  TestConnectDisconnectLifecycle();
  TestAsyncCallbacksXrun();
  TestConnectDisconnectCyclesAsyncCallbacks();
  TestDisconnectWaitsAsyncCallbacks();
}
