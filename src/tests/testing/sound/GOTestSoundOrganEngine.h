/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDORGANENGINE_H
#define GOTESTSOUNDORGANENGINE_H

#include "GOTestSoundOrganEngineBase.h"

/*
 * Tests for GOSoundOrganEngine using direct ProcessAudioCallback calls,
 * without GOSoundCallbackConnector. SetUsed is managed manually.
 */
class GOTestSoundOrganEngine : public GOTestSoundOrganEngineBase {
private:
  static const std::string TEST_NAME;

  /* BuildAndStartEngine + SetUsed(true) + SetStreaming(true). */
  GOSoundOrganEngine &BuildStartAndConnectEngine(
    unsigned nAudioGroups, unsigned nAuxThreads, unsigned nOutputs);

  /* SetStreaming(false) + SetUsed(false) + StopAndDestroyEngine. */
  void DisconnectStopAndDestroyEngine();

  /* Build→SetUsed→SetStreaming→PAC×5→SetStreaming(false)→SetUsed(false)→Stop.
   */
  void TestSingleOutputLifecycle();

  /* 5 periods of PAC(0)+PAC(1); checks didAdvance per output. */
  void TestTwoOutputsLifecycleWith(unsigned nAudioGroups);
  void TestTwoOutputsLifecycle();
  void TestTwoGroupsTwoOutputsLifecycle();

  /* 3 cycles of WORKING↔USED; checks IsWorking&&!IsUsed at each step. */
  void TestSetUsedTransitions();

  /*
   * 100 build/stop cycles; 2 threads both call PAC(0) concurrently (xrun).
   * Threads start after SetUsed(true) and stop before SetUsed(false).
   */
  void TestBuildStopCyclesAsyncCallbacksXrun();

  /*
   * 100 cycles over 8 engine configurations; one thread per output calls
   * PAC(outputI) for N_PERIODS iterations.
   */
  void TestMultipleConfigsAsyncCallbacks();

  /*
   * Verifies that SetStreaming(false) unblocks PAC(0) blocked at [W1] during
   * a simulated disconnect: with 2 outputs, an xrun on output 0 causes PAC(0)
   * to block at [W1]; SetStreaming(false) broadcasts [W1], PAC(0) exits early,
   * and the 1-second disconnect wait completes without timeout.
   */
  void TestDisconnectWithXrunDeadlock();

  /*
   * Verifies that SetStreaming(true) resets the per-period counters
   * (m_NCallbacksEntered, m_NCallbacksFinished): with 3 outputs, the first
   * streaming session processes only outputs 0 and 1 (incomplete period),
   * leaving the counters dirty. Without the reset, the second session
   * deadlocks because the dirty counters cause output 0 alone to advance
   * the period, leaving outputs 1 and 2 with wasProcessedInCurrentPeriod=true
   * and no callback to reset them.
   */
  void TestReconnectAfterMidPeriodDisconnect();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDORGANENGINE_H */
