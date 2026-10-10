/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDRESAMPLE_H
#define GOTESTSOUNDRESAMPLE_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundResample::ResampleBlockVariableRatePlanar() and
 * RingPlanarFrameVector, the variable-rate resampling path added for the
 * upcoming pitch-modulated tremulant (#709). GOSoundVibratoProcessor is the
 * only production consumer, and it only ever calls the planar entry point
 * (it always writes into a GOSoundBufferPlanarMutable), so these tests
 * check that path directly rather than through an interleaved variant that
 * doesn't exist in this class.
 */
class GOTestSoundResample : public GOTest {
private:
  static const std::string TEST_NAME;

  /** ResampleBlockVariableRatePlanar() fed a constant-increment array must
   * match ResampleBlock()'s own output (within a small relative tolerance -
   * see the test's own comment) and final ResamplingPosition state
   * exactly, for both LINEAR and POLYPHASE. */
  void TestConstantRateEquivalence();

  /** ResampleBlockVariableRatePlanar() over a hand-picked varying-increment
   * array, checked against an independently hand-computed
   * index/fraction/output trajectory (LINEAR only, exact arithmetic). */
  void TestVaryingRateCorrectness();

  /** RingPlanarFrameVector reads, confined to [0, nLogicalFrames) and not
   * needing the mirrored tail, match an equivalent PtrFrameVector's
   * reads of the same logical values. */
  void TestRingNonWrappingReads();

  /** RingPlanarFrameVector reads whose nPoints-wide window crosses the
   * logical-frame-count boundary, including a read starting exactly at
   * the seam, are answered correctly from the mirrored tail. */
  void TestRingMirroredSeamReads();

  /** RingPlanarFrameVector::NormalizePosition(): driven through many
   * Inc()/NormalizePosition() call pairs (mimicking ResampleBlock()'s/
   * ResampleBlockVariableRatePlanar()'s own per-frame sequence), including
   * several full wraps, GetIndex() must stay below nLogicalFrames and
   * match an independently computed (unbounded index) % nLogicalFrames
   * trajectory. */
  void TestRingNormalizePosition();

  /** ResampleBlockVariableRatePlanar() with PolyphaseResampler, reading
   * stereo through a RingPlanarFrameVector whose window crosses the ring's
   * seam, must match the same function reading the equivalent
   * manually-unwrapped source via PtrFrameVector - the combination a
   * modulated delay line uses. Stereo also exercises the ring's
   * per-channel stride (channel * nPhysicalFrames) through the resampler,
   * which the mono-only correctness tests above never touch. */
  void TestRingResamplerIntegration();

  /** rateToFractionIncrement() as constexpr: usable in a static_assert and
   * equal to the former roundf() version on the rates the code uses and on a
   * grid of non-negative rates. */
  void TestRateToFractionIncrement();

  /** getIndexIncrementByUnits() must equal the real index growth of a
   * ResamplingPosition after random Inc() sequences, for several starting
   * fractions. */
  void TestGetIndexIncrementByUnits();

  /** computeMinNUnitsToReach() and computeMaxNUnitsToReach() must land
   * exactly on index + k with a zero fraction, and the neighbouring sums must
   * be on the right side of the bound. */
  void TestComputeNUnitsToReach();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDRESAMPLE_H */
