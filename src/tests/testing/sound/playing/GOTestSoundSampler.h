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
 * Tests the resampling increment handling of GOSoundSampler: the increment of
 * a playing stream follows the tuning of its provider.
 */
class GOTestSoundSampler : public GOTest {
private:
  static const std::string TEST_NAME;

  /** At the target tuning the increment is the target one, and a changed
   * tuning scales the target increment by the ratio of the tunings. */
  void TestIncrementFollowsTuning();

  /** The increment depends on the target pair and the tuning only: many
   * changes of the tuning and a return to the target tuning do not accumulate
   * rounding errors. */
  void TestIncrementDoesNotDrift();

  /** InitResamplerIncrement() makes the new increment both the target and the
   * applied one, so a stream that takes over the rate of another one is scaled
   * from its own initial increment. */
  void TestInitResamplerIncrementRebasesTarget();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDSAMPLER_H */
