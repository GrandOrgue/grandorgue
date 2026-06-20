/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTRELEASEALIGNTABLE_H
#define GOTESTRELEASEALIGNTABLE_H

#include <string>

#include "GOTest.h"

class GOTestReleaseAlignTable : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestComputeBucketIndexNormal();
  void TestComputeBucketIndexBelowRange();
  void TestComputeBucketIndexAboveRange();
  void TestComputeBucketIndexZeroMaxValue();
  void TestComputeBucketIndexBoundaries();

  /**
   * Regression test for the original bug: when only one cell in the table is
   * filled, its direct vertical neighbor (same amplitude, other derivative row)
   * must be found. The old algorithm never visited offset sk=0, so this case
   * caused assert(nearestFilledCell) to fire.
   */
  void TestBugRegression();

  /**
   * Verifies that the derivative dimension is the outer loop: when target and
   * a filled cell share the same derivative row, that cell is preferred over
   * an equidistant cell in the other row.
   */
  void TestSameRowPreference();

  /**
   * When the only filled cell is at the far corner of the table, every other
   * cell must still be able to reach it.
   */
  void TestDistantSingleCell();

  /**
   * An entirely empty table must return std::nullopt rather than crash.
   */
  void TestEmptyTable();

  /**
   * The function must return the target cell itself when it is already marked
   * filled (offset 0,0 is visited first).
   */
  void TestCellFindsItself();

  /**
   * Within the same derivative row, positive amplitude offsets are visited
   * before negative offsets of the same magnitude (+2 before -2).
   */
  void TestAmplitudeOrdering();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTRELEASEALIGNTABLE_H */
