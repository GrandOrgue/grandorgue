/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDWINDCHESTTASK_H
#define GOTESTSOUNDWINDCHESTTASK_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundWindchestTask's processing-chain ownership (Stage 5):
 * the chain it is constructed with is stored and exposed by GetChain()
 * unchanged, and DoRun() keeps that chain's parameters up to date.
 * Amplitude computation itself (the pre-Stage-5 behavior) is left to
 * GOTestSoundWindchestGroupTask and the engine-level tests.
 */
class GOTestSoundWindchestTask : public GOTest {
private:
  static const std::string TEST_NAME;

  /** GetChain() never returns null, for both an empty chain and one with
   * processors. */
  void TestGetChainNeverNull();

  /** The chain the task was constructed with has the processors that were
   * added to it, in order - proving the constructor's chain argument is
   * exposed unchanged. */
  void TestChainReflectsConstructedChain();

  /** DoRun() (driven by GetAmplitude()) calls the chain's
   * EnsureParametersUpToDate(), once per Run(). */
  void TestDoRunCallsEnsureParametersUpToDate();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDWINDCHESTTASK_H */
