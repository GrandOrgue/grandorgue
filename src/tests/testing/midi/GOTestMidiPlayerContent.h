/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTMIDIPLAYERCONTENT_H
#define GOTESTMIDIPLAYERCONTENT_H

#include "GOTest.h"

class GOTestMidiPlayerContent : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestIsMidiInputNumberMappingUsable();
  void TestComputeManualChannelsPedalFirst();
  void TestComputeManualChannelsPedalLast();
  void TestComputeManualChannelsUseInputNumber();
  void TestComputeManualChannelsUseInputNumberFallback();
  void TestHasNativeHeader();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTMIDIPLAYERCONTENT_H */
