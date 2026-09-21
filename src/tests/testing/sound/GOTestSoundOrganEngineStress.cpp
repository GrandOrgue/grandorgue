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
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/interfaces/GOSoundCallbackConnector.h"

#include "GOTestScope.h"

const std::string GOTestSoundOrganEngineStress::TEST_NAME
  = "GOTestSoundOrganEngineStress";

void GOTestSoundOrganEngineStress::RunCallbackThreads(
  std::atomic_bool &isRunning, std::vector<std::thread> &outThreads) {
  auto makeThread = [&](unsigned outputI) {
    return std::thread([&, outputI]() {
      while (isRunning.load()) {
        GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
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

  // The first ConnectToEngine() opens the round StartEngine() left pending
  // (m_IsToStartPeriod) with no period behind it; exclude it from the
  // baseline below so the invariant only has to cover genuine graceful-stop
  // slack.
  m_connector.ConnectToEngine(engine);
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  m_connector.DisconnectFromEngine(engine);

  const uint64_t timeAtStart = engine.GetTime();
  const uint64_t roundAtStart
    = engine.GetScheduler().GetRoundCounter().GetRoundNumber();

  for (unsigned cycleI = 0; cycleI < 99; ++cycleI) {
    m_connector.ConnectToEngine(engine);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    m_connector.DisconnectFromEngine(engine);
  }

  isRunning.store(false);
  for (std::thread &t : threads)
    t.join();

  /* Period-accounting invariant, checked here after every callback thread
     has joined and the last DisconnectFromEngine() has drained them, so
     nothing can still be advancing either counter: the round counter can
     lag the clock by at most one buffer - a graceful stop finishes
     (FinishPeriod(), advancing the clock) before its round is started
     (StartPeriod(), advancing the round counter), and the two can happen
     arbitrarily far apart in time (here, possibly never within this test:
     the last cycle's disconnect may leave m_IsToStartPeriod set with no
     further reconnect to consume it). It can never lag by more than one,
     since only one graceful stop's round can be pending at a time, and it
     can never lead the clock at all - StartPeriod() never runs without a
     FinishPeriod() either just before it (same call) or already having run
     earlier for it (a pending flag). StopEngine() below neither advances nor
     catches up either counter, so this check does not need to run before
     it. */
  const uint64_t clockDelta = engine.GetTime() - timeAtStart;
  const uint64_t roundDelta
    = engine.GetScheduler().GetRoundCounter().GetRoundNumber() - roundAtStart;

  GOAssert(
    clockDelta >= roundDelta * N_SAMPLES_PER_BUFFER
      && clockDelta <= (roundDelta + 1) * N_SAMPLES_PER_BUFFER,
    "the round counter must track the clock within one pending graceful "
    "period - a wider mismatch means a period was accounted twice, not at "
    "all, or its round was started before it was finished");

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

    // Connecting opens the round StartEngine() (inside BuildAndStartEngine())
    // left pending, with no period behind it; baseline starts here so the
    // invariant below only has to cover this cycle's own graceful-stop slack.
    const uint64_t timeAtStart = engine.GetTime();
    const uint64_t roundAtStart
      = engine.GetScheduler().GetRoundCounter().GetRoundNumber();

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    m_connector.DisconnectFromEngine(engine);

    // Same period-accounting invariant as TestConnectDisconnectCycles, per
    // cycle: DisconnectFromEngine() has just drained every callback thread
    // (they idle on p_OrganEngine == nullptr while the engine is being
    // rebuilt), so nothing can be mid-round; the only possible lag here is
    // this cycle's own disconnect, if it hit a graceful boundary with no
    // reconnect yet to start its round.
    const uint64_t clockDelta = engine.GetTime() - timeAtStart;
    const uint64_t roundDelta
      = engine.GetScheduler().GetRoundCounter().GetRoundNumber() - roundAtStart;

    GOAssert(
      clockDelta >= roundDelta * N_SAMPLES_PER_BUFFER
        && clockDelta <= (roundDelta + 1) * N_SAMPLES_PER_BUFFER,
      "the round counter must track the clock within one pending graceful "
      "period - a wider mismatch means a period was accounted twice, not at "
      "all, or its round was started before it was finished");

    StopAndDestroyEngine();
  }

  isRunning.store(false);
  for (std::thread &t : threads)
    t.join();
}

void GOTestSoundOrganEngineStress::run() {
  GO_RUN_TEST(TestConnectDisconnectCycles())
  GO_RUN_TEST(TestBuildStopCycles())
}
