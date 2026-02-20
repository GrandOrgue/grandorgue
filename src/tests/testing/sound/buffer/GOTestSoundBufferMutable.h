/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDMUTABLEBUFFER_H
#define GOTESTSOUNDMUTABLEBUFFER_H

#include "GOTestSoundBufferBase.h"

class GOTestSoundBufferMutable : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void AssertAllItemsEqual(
    const std::string &context,
    float expectedValue,
    const GOSoundBuffer &buffer);
  void AssertBuffersDataEqual(
    const std::string &context,
    const GOSoundBuffer &expectedBuffer,
    const GOSoundBuffer &buffer);
  void AssertChannelItemEqual(
    const std::string &context,
    unsigned frameIndex,
    unsigned channelIndex,
    float expectedValue,
    float gotValue);
  void AssertChannelItemNear(
    const std::string &context,
    unsigned frameIndex,
    unsigned channelIndex,
    float expectedValue,
    float gotValue);
  void AssertChannelEqual(
    const std::string &context,
    const GOSoundBuffer &dstBuffer,
    unsigned dstChannelI,
    const GOSoundBuffer &srcBuffer,
    unsigned srcChannelI);

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
