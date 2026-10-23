/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTGUILAYOUTENGINE_H
#define GOTESTGUILAYOUTENGINE_H

#include "GOTest.h"

/**
 * Tests the pure arithmetic of the grids of GOGUILayoutEngine: it needs no
 * metrics and no display
 */
class GOTestGUILayoutEngine : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestGridBlitPosition();
  void TestCentredPosition();
  void TestEnclosureAreaX();
  void TestJambWidths();
  void TestJambXs();
  void TestCenterExpansion();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTGUILAYOUTENGINE_H */
