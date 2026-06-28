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

void GOTestReleaseAlignTable::run() {
  TestComputeBucketIndexNormal();
  TestComputeBucketIndexBelowRange();
  TestComputeBucketIndexAboveRange();
  TestComputeBucketIndexZeroMaxValue();
  TestComputeBucketIndexBoundaries();
}
