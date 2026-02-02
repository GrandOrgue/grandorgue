/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERMANAGED_H
#define GOTESTSOUNDBUFFERMANAGED_H

#include "GOTest.h"

#include <string>

class GOTestSoundBufferManaged : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestDefaultConstructor();
  void TestConstructorWithDimensions();
  void TestCopyConstructorFromBuffer();
  void TestCopyConstructorFromManaged();
  void TestMoveConstructor();
  void TestCopyAssignmentFromBuffer();
  void TestCopyAssignmentFromManaged();
  void TestMoveAssignment();
  void TestResize();
  void TestResizeNoReallocation();
  void TestSwap();
  void TestComplexOperations();
  void TestSubBufferCompatibility();
  void TestInvalidBufferOperations();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDBUFFERMANAGED_H */
