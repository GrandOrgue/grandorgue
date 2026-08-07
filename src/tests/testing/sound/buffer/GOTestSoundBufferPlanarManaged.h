/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERPLANARMANAGED_H
#define GOTESTSOUNDBUFFERPLANARMANAGED_H

#include "GOTestSoundBufferBase.h"

class GOTestSoundBufferPlanarManaged : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestDefaultConstructor();
  void TestConstructorWithDimensions();
  void TestCopyConstructorFromView();
  void TestCopyConstructorFromManaged();
  void TestMoveConstructor();
  void TestCopyAssignmentFromView();
  void TestCopyAssignmentFromManaged();
  void TestMoveAssignment();
  void TestResize();
  void TestResizeNoReallocation();
  void TestSwap();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDBUFFERPLANARMANAGED_H */
