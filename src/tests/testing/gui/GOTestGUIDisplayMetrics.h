/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTGUIDISPLAYMETRICS_H
#define GOTESTGUIDISPLAYMETRICS_H

#include "GOTest.h"

/**
 * Tests the static arithmetic of GOGUIDisplayMetrics: it needs no object and
 * no display
 */
class GOTestGUIDisplayMetrics : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestCellOffset();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTGUIDISPLAYMETRICS_H */
