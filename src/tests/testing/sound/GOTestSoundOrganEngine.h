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

  /*
   * StopEngine() must not touch the sampler pool: a sample started before
   * Stop is still checked out (GetUsedSamplerCount() unchanged) and can
   * still be mixed after the following StartEngine() resumes.
   */
  void TestStopStartResumePreservesSamplers();

  /*
   * DestroyEngine() itself does not touch the pool - GetUsedSamplerCount()
   * stays at whatever it was before Destroy; reclaiming (GetUsedSamplerCount()
   * back to 0) happens on the following BuildEngine(). A polyphony limit
   * lowered after Destroy still takes effect on that next BuildEngine(),
   * since the pool's shrink-on-return happens there rather than in
   * DestroyEngine().
   */
  void TestDestroyRebuildReclaimsAndResizesPool();

  /* AudioGroupRoutingChange::IsEmpty() is true default-constructed, false
   * once PrepareSoundRoutingFor() actually builds a task. */
  void TestAudioGroupRoutingChangeIsEmpty();

  /*
   * HasSoundRoutingFor() is false for a pair with no task and true once
   * PrepareSoundRoutingFor() has built its grid cells - even before
   * CommitSoundRoutingFor() registers them with the scheduler, since Has()
   * only reflects grid population.
   */
  void TestHasSoundRoutingFor();

  /*
   * PrepareSoundRoutingFor() for a pair with no existing cell builds it
   * (and its detached-release cell) without touching the scheduler or any
   * GOSoundGroupTask while the engine keeps running; CommitSoundRoutingFor()
   * under a suspend/resume then makes the pair actually usable - a sample
   * started on it afterwards does not hit GetWindchestGroupTask()'s
   * assert(pTask). A second PrepareSoundRoutingFor() for the same pair,
   * even without an intervening Commit, does not try to build again (the
   * grid cell built by the first call already makes Has() true).
   */
  void TestPrepareAndCommitSoundRoutingFor();

  /*
   * Starts a real GOSoundProviderSynthedTrem sample and checks that
   * non-silent audio reaches the final planar output buffer after a few
   * periods - a content-level canary for the windchest-group merge
   * (DeinterleaveFrom()/AddDeinterleavedFrom(), Stage 3's planar rewrite):
   * a channel/frame transpose bug there would very likely silence the
   * output, which none of the round-state-only checks elsewhere in this
   * suite would catch. Single-threaded (nAuxThreads=0) on purpose: mixing
   * direct/synchronous ProcessAudioCallback() calls with a live aux
   * scheduler thread is exercised only by the dedicated async-callback
   * tests, which drive every output from its own thread - not by a single
   * calling thread looping alone, which deadlocked when tried here.
   */
  void TestSamplerAudioReachesPlanarOutput();

  /*
   * BuildEngine() fills each windchest's chain from its own enclosures:
   * a windchest with no enclosures gets an empty chain, and a windchest
   * with N enclosures gets N GOSoundShelfFilterProcessor/
   * GOSoundEnclosureShelfMapper pairs - one per enclosure, regardless of
   * configured attenuation or position (issue #717).
   */
  void TestChainReflectsWindchestEnclosures();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDORGANENGINE_H */
