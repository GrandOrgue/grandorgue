/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDMUTABLEBUFFERMONO_H
#define GOTESTSOUNDMUTABLEBUFFERMONO_H

#include "GOTest.h"

class GOTestSoundBufferMutableMono : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestConstructorAndBasicProperties();
  void TestGetSubBuffer();
  void TestCopyChannelFrom();
  void TestCopyChannelTo();
  void TestInvalidBuffer();
  void TestEdgeCases();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDMUTABLEBUFFERMONO_H */
