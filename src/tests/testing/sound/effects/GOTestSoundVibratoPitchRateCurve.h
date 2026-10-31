/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDVIBRATOPITCHRATECURVE_H
#define GOTESTSOUNDVIBRATOPITCHRATECURVE_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundVibratoPitchRateCurve: the constants,
 * applyAffineTransform(), normalisePitchRateCurve(), Build(), BuildInternal(),
 * moving and GetChunkDescription(). Friended by the curve for direct access.
 */
class GOTestSoundVibratoPitchRateCurve : public GOTest {
private:
  static const std::string TEST_NAME;

  /** The rate-range constants are consistent with each other, with the
   * resampler's units and with uint16_t. */
  void TestConstants();

  /** applyAffineTransform(): a plain shift, exact rounding, the clamp, the
   * returned sum, an empty array. */
  void TestApplyAffineTransform();

  /** normalisePitchRateCurve() on random curves and targets: the sum is exact
   * and every rate stays in the range. */
  void TestNormaliseRandom();

  /** normalisePitchRateCurve() pressing branch: a deep square wave keeps its
   * shape, the sum is exact and the rates stay in the range. */
  void TestNormalisePressed();

  /** normalisePitchRateCurve() on the examples of the design: constants and
   * very short curves. */
  void TestNormaliseExamples();

  /** Build() layout: sizes, loop, tiled tail, the exact loop sum. */
  void TestBuildLayout();

  /** Build() of a deep curve: every rate in the range, the exact loop sum, the
   * shape is kept. */
  void TestBuildDeepCurve();

  /** Build() clamps out-of-range deviations, treats NaN as no deviation and
   * accepts both bounds exactly. */
  void TestBuildClampsDeviations();

  /** BuildInternal(..., false) gives the plain converted rates. */
  void TestBuildInternalNotNormalised();

  /** Build() normalises a curve whose input mean is not 0: the mean rate is
   * exactly 1. */
  void TestBuildNonZeroMean();

  /** Destroy() empties a built curve, which can then be built again. */
  void TestDestroy();

  /** A curve can be moved, emptied and rebuilt. */
  void TestMoveAndEmpty();

  /** GetChunkDescription(): the chunk points at the cursor, the precomputed
   * sum equals the direct sum at every start index. */
  void TestChunkDescription();

  /** The empty chunk {nullptr, 0, 0} has no rates. */
  void TestEmptyChunk();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDVIBRATOPITCHRATECURVE_H */
