/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDMUTABLEBUFFER_H
#define GOTESTSOUNDMUTABLEBUFFER_H

#include "GOTest.h"

class GOSoundBuffer;

class GOTestSoundBufferMutable : public GOTest {
private:
  static const std::string TEST_NAME;

  void AssertItemEqual(
    const std::string &context,
    unsigned itemIndex,
    float expectedValue,
    float gotValue);
  void AssertAllItemsEqual(
    const std::string &context,
    float expectedValue,
    const GOSoundBuffer &buffer);
  void AssertBuffersDataEqual(
    const std::string &context,
    const GOSoundBuffer &expectedBuffer,
    const GOSoundBuffer &buffer);
  void AssertChannelNear(
    const std::string &context,
    unsigned frameIndex,
    unsigned channelIndex,
    float expectedValue,
    float gotValue);

  void TestInheritanceAndMutableAccess();
  void TestFillWithSilence();
  void TestCopyFrom();
  void TestAddFrom();
  void TestAddFromWithCoefficient();
  void TestMutableGetSubBuffer();
  void TestCompatibilityChecks();
  void TestComplexOperations();
  void TestCopyChannelFrom();
  void TestAddChannelFrom();
  void TestAddChannelFromWithCoefficient();
  void TestCrossChannelOperations();
  void TestChannelOperationsWithSubBuffers();
  void TestInvalidChannelIndices();
  void TestSingleChannelBuffer();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDMUTABLEBUFFER_H */
