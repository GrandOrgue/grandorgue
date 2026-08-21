/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERPLANAR_H
#define GOTESTSOUNDBUFFERPLANAR_H

#include "GOTestSoundBufferBase.h"

class GOTestSoundBufferPlanar : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestConstructorAndBasicProperties();
  void TestGetChannelBuffer();
  void TestInterleaveTo();
  void TestInvalidBuffer();
  void TestEdgeCases();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDBUFFERPLANAR_H */
