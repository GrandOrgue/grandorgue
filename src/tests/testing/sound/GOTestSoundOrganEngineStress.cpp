/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundOrganEngineStress.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "sound/GOSoundOrganEngine.h"
#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/interfaces/GOSoundCallbackConnector.h"

const std::string GOTestSoundOrganEngineStress::TEST_NAME
  = "GOTestSoundOrganEngineStress";

void GOTestSoundOrganEngineStress::RunCallbackThreads(
  std::atomic_bool &isRunning, std::vector<std::thread> &outThreads) {
  auto makeThread = [&](unsigned outputI) {
    return std::thread([&, outputI]() {
      while (isRunning.load()) {
        GO_DECLARE_LOCAL_SOUND_BUFFER(
          buf, N_OUTPUT_CHANNELS, N_SAMPLES_PER_BUFFER);

        m_connector.AudioCallback(outputI, buf);
      }
    });
  };

  outThreads.push_back(makeThread(0));
  outThreads.push_back(makeThread(0));
  outThreads.push_back(makeThread(1));
  outThreads.push_back(makeThread(2));
  outThreads.push_back(makeThread(2));
}

void GOTestSoundOrganEngineStress::TestConnectDisconnectCycles() {
  GOSoundOrganEngine &engine = BuildAndStartEngine(
    /* nAudioGroups */ 2, /* nAuxThreads */ 3, /* nOutputs */ 3);

  std::atomic_bool isRunning{true};
  std::vector<std::thread> threads;

  RunCallbackThreads(isRunning, threads);

  for (unsigned cycleI = 0; cycleI < 100; ++cycleI) {
    m_connector.ConnectToEngine(engine);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    m_connector.DisconnectFromEngine(engine);
  }

  isRunning.store(false);
  for (std::thread &t : threads)
    t.join();

  StopAndDestroyEngine();
}

void GOTestSoundOrganEngineStress::TestBuildStopCycles() {
  std::atomic_bool isRunning{true};
  std::vector<std::thread> threads;

  RunCallbackThreads(isRunning, threads);

  for (unsigned cycleI = 0; cycleI < 100; ++cycleI) {
    GOSoundOrganEngine &engine = BuildAndStartEngine(
      /* nAudioGroups */ 2, /* nAuxThreads */ 3, /* nOutputs */ 3);

    m_connector.ConnectToEngine(engine);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    m_connector.DisconnectFromEngine(engine);

    StopAndDestroyEngine();
  }

  isRunning.store(false);
  for (std::thread &t : threads)
    t.join();
}

void GOTestSoundOrganEngineStress::run() {
  TestConnectDisconnectCycles();
  TestBuildStopCycles();
}
