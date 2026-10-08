/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERMONO_H
#define GOTESTSOUNDBUFFERMONO_H

#include "GOTestSoundBufferBase.h"

class GOTestSoundBufferMono : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestConstructorAndBasicProperties();
  void TestConvertingConstructor();
  void TestGetSubBuffer();
  void TestInvalidBuffer();
  void TestEdgeCases();
  void TestRoundTripWithPlanarGetChannelBuffer();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDBUFFERMONO_H */
