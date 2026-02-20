/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERMANAGED_H
#define GOTESTSOUNDBUFFERMANAGED_H

#include "GOTest.h"

#include <string>

#include "sound/buffer/GOSoundBuffer.h"

class GOTestSoundBufferManaged : public GOTest {
private:
  static const std::string TEST_NAME;

  void AssertDataEquals(
    const std::string &context,
    const GOSoundBuffer::Item *expectedData,
    unsigned nItems,
    const GOSoundBuffer &buffer);
  void TestDeepCopy(
    const std::string &context,
    GOSoundBuffer::Item *sourceData,
    const GOSoundBuffer &buffer);

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
