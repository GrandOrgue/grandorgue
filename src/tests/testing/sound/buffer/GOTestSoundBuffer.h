/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFER_H
#define GOTESTSOUNDBUFFER_H

#include "GOTest.h"

class GOTestSoundBuffer : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestConstructorAndBasicProperties();
  void TestGetNUnits();
  void TestGetUnitOffset();
  void TestGetSubBuffer();
  void TestInvalidBuffer();
  void TestEdgeCases();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDBUFFER_H */
