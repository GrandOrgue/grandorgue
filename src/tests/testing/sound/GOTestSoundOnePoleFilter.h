/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDONEPOLEFILTER_H
#define GOTESTSOUNDONEPOLEFILTER_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundOnePoleFilter::computeCoeffs()/processSample() (the pure
 * static core left after Stage 5's GOSoundFilter -> GOSoundOnePoleFilter
 * rename+restructure).
 */
class GOTestSoundOnePoleFilter : public GOTest {
private:
  static const std::string TEST_NAME;

  /** Every isNoop-triggering edge case (sampleRate == 0, TYPE_NONE, gain ==
   * 0 for both shelf types) yields the exact identity Coeffs (b0=1, b1=0,
   * a1=0, isNoop=true), not all-zero. */
  void TestComputeCoeffsIdentityEdgeCases();

  /** Every non-noop Type produces a non-identity, isNoop==false Coeffs. */
  void TestComputeCoeffsPerType();

  /** processSample() follows the documented recurrence for a couple of
   * hand-computed inputs. */
  void TestProcessSampleRecurrence();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDONEPOLEFILTER_H */
