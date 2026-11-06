/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDSAMPLER_H
#define GOTESTSOUNDSAMPLER_H

#include <string>

#include "GOTest.h"

/**
 * Tests how GOSoundSampler follows the volume factor and the tuning factor of
 * its provider while it is playing.
 */
class GOTestSoundSampler : public GOTest {
private:
  static const std::string TEST_NAME;

  /** At the initial tuning factor the increment is the initial one, and a
   * changed tuning factor scales the initial increment by the ratio of the
   * factors, within MIN_FRACTION_INCREMENT..MAX_FRACTION_INCREMENT. */
  void TestTuningFactorFollowsProvider();

  /** The increment depends on the initial values and the tuning factor only:
   * many changes of the tuning factor and a return to the initial one do not
   * accumulate rounding errors. */
  void TestTuningFactorDoesNotDrift();

  /** EnsureTuningFactor() recomputes the increment only when the tuning factor
   * differs from the last applied one. */
  void TestTuningFactorIsEnsuredOnlyOnChange();

  /** InitResamplerIncrement() makes the new values both the initial and the
   * applied ones, so a stream that takes over the rate of another one is scaled
   * from its own initial increment. */
  void TestInitResamplerIncrementRebasesInitial();

  /** The volume ratio is computed from the initial volume factor, so a return
   * to it gives exactly 1.0. */
  void TestVolumeFactorFollowsProvider();

  /** EnsureVolumeFactor() recomputes the ratio only when the volume factor
   * differs from the last applied one. */
  void TestVolumeFactorIsEnsuredOnlyOnChange();

  /** A sampler started with the volume factor 0 keeps the ratio 1.0. */
  void TestSilentSamplerIsNotRaised();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDSAMPLER_H */
