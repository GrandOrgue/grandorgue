/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestReleaseAlignTable.h"

#include <format>

#include "sound/playing/GOSoundReleaseAlignTable.h"

const std::string GOTestReleaseAlignTable::TEST_NAME
  = "GOTestReleaseAlignTable";

void GOTestReleaseAlignTable::TestComputeBucketIndexNormal() {
  // shiftedValue at midpoint of [0, 2*maxValue) maps to center bucket
  const unsigned result
    = GOSoundReleaseAlignTable::computeBucketIndex(10, 10u, 4u);

  GOAssert(
    result == 2u,
    std::format("center value should map to bucket 2, got: {}", result));
}

void GOTestReleaseAlignTable::TestComputeBucketIndexBelowRange() {
  // negative shiftedValue clamps to bucket 0
  const unsigned result
    = GOSoundReleaseAlignTable::computeBucketIndex(-5, 10u, 4u);

  GOAssert(
    result == 0u,
    std::format("negative value should map to bucket 0, got: {}", result));
}

void GOTestReleaseAlignTable::TestComputeBucketIndexAboveRange() {
  // shiftedValue >= 2*maxValue clamps to last bucket
  const unsigned result
    = GOSoundReleaseAlignTable::computeBucketIndex(20, 10u, 4u);

  GOAssert(
    result == 3u,
    std::format("value above range should map to bucket 3, got: {}", result));
}

void GOTestReleaseAlignTable::TestComputeBucketIndexZeroMaxValue() {
  // maxValue == 0 returns midpoint bucket
  const unsigned result
    = GOSoundReleaseAlignTable::computeBucketIndex(0, 0u, 8u);

  GOAssert(
    result == 4u,
    std::format("zero maxValue should return midpoint 4, got: {}", result));
}

void GOTestReleaseAlignTable::TestComputeBucketIndexBoundaries() {
  // lower boundary: shiftedValue == 0 maps to bucket 0
  const unsigned resultLow
    = GOSoundReleaseAlignTable::computeBucketIndex(0, 10u, 4u);

  GOAssert(
    resultLow == 0u,
    std::format("lower boundary 0 should map to bucket 0, got: {}", resultLow));

  // upper boundary: shiftedValue == 2*maxValue-1 maps to last bucket
  const unsigned resultHigh
    = GOSoundReleaseAlignTable::computeBucketIndex(19, 10u, 4u);

  GOAssert(
    resultHigh == 3u,
    std::format(
      "upper boundary 19 should map to bucket 3, got: {}", resultHigh));
}

static void clearTable(
  bool table[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES]) {
  for (unsigned derivI = 0; derivI < PHASE_ALIGN_DERIVATIVES; derivI++)
    for (unsigned ampI = 0; ampI < PHASE_ALIGN_AMPLITUDES; ampI++)
      table[derivI][ampI] = false;
}

void GOTestReleaseAlignTable::TestBugRegression() {
  /* Original bug: the old sk-formula never generated sk=0, so cell (1,16)
   * couldn't find its only filled neighbor at (0,16). */
  bool table[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

  clearTable(table);
  table[0][16] = true;

  const auto result
    = GOSoundReleaseAlignTable::findNearestFilledCell(table, {1, 16});

  GOAssert(result.has_value(), "TestBugRegression: expected a result");
  GOAssert(
    result->derivI == 0 && result->ampI == 16,
    std::format(
      "TestBugRegression: expected (0,16), got ({},{})",
      result->derivI,
      result->ampI));
}

void GOTestReleaseAlignTable::TestSameRowPreference() {
  /* Both (0,0) and (1,0) are filled; target (0,1) is in the same derivative
   * row as (0,0), so (0,0) must be returned first. */
  bool table[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

  clearTable(table);
  table[0][0] = true;
  table[1][0] = true;

  const auto result
    = GOSoundReleaseAlignTable::findNearestFilledCell(table, {0, 1});

  GOAssert(result.has_value(), "TestSameRowPreference: expected a result");
  GOAssert(
    result->derivI == 0 && result->ampI == 0,
    std::format(
      "TestSameRowPreference: expected (0,0), got ({},{})",
      result->derivI,
      result->ampI));
}

void GOTestReleaseAlignTable::TestDistantSingleCell() {
  /* Single filled cell at the far corner (1,31); target is (0,0). */
  bool table[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

  clearTable(table);
  table[1][31] = true;

  const auto result
    = GOSoundReleaseAlignTable::findNearestFilledCell(table, {0, 0});

  GOAssert(result.has_value(), "TestDistantSingleCell: expected a result");
  GOAssert(
    result->derivI == 1 && result->ampI == 31,
    std::format(
      "TestDistantSingleCell: expected (1,31), got ({},{})",
      result->derivI,
      result->ampI));
}

void GOTestReleaseAlignTable::TestEmptyTable() {
  bool table[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

  clearTable(table);

  const auto result
    = GOSoundReleaseAlignTable::findNearestFilledCell(table, {0, 0});

  GOAssert(!result.has_value(), "TestEmptyTable: expected std::nullopt");
}

void GOTestReleaseAlignTable::TestCellFindsItself() {
  /* The target cell itself is filled; offset (0,0) is visited first. */
  bool table[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

  clearTable(table);
  table[0][5] = true;

  const auto result
    = GOSoundReleaseAlignTable::findNearestFilledCell(table, {0, 5});

  GOAssert(result.has_value(), "TestCellFindsItself: expected a result");
  GOAssert(
    result->derivI == 0 && result->ampI == 5,
    std::format(
      "TestCellFindsItself: expected (0,5), got ({},{})",
      result->derivI,
      result->ampI));
}

void GOTestReleaseAlignTable::TestAmplitudeOrdering() {
  /* Cells at (0,3) and (0,7) are equidistant from target (0,5) in amplitude.
   * The algorithm visits +delta before -delta of the same magnitude, so
   * (0,7) (+2) must be returned before (0,3) (-2). */
  bool table[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

  clearTable(table);
  table[0][3] = true;
  table[0][7] = true;

  const auto result
    = GOSoundReleaseAlignTable::findNearestFilledCell(table, {0, 5});

  GOAssert(result.has_value(), "TestAmplitudeOrdering: expected a result");
  GOAssert(
    result->derivI == 0 && result->ampI == 7,
    std::format(
      "TestAmplitudeOrdering: expected (0,7), got ({},{})",
      result->derivI,
      result->ampI));
}

void GOTestReleaseAlignTable::run() {
  TestComputeBucketIndexNormal();
  TestComputeBucketIndexBelowRange();
  TestComputeBucketIndexAboveRange();
  TestComputeBucketIndexZeroMaxValue();
  TestComputeBucketIndexBoundaries();
  TestBugRegression();
  TestSameRowPreference();
  TestDistantSingleCell();
  TestEmptyTable();
  TestCellFindsItself();
  TestAmplitudeOrdering();
}
