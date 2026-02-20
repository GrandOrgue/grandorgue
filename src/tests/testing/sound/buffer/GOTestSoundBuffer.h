/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFER_H
#define GOTESTSOUNDBUFFER_H

#include "GOTestSoundBufferBase.h"

class GOTestSoundBuffer : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void AssertGetNItems(
    unsigned nChannels, unsigned nFrames, unsigned expectedValue);
  void AssertGetItemIndex(
    const GOSoundBuffer &buffer,
    unsigned frameIndex,
    unsigned channelIndex,
    unsigned expectedValue);

  void TestConstructorAndBasicProperties();
  void TestGetNItems();
  void TestGetItemIndex();
  void TestGetSubBuffer();
  void TestInvalidBuffer();
  void TestEdgeCases();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDBUFFER_H */
