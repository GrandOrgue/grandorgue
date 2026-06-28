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

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTRELEASEALIGNTABLE_H */
