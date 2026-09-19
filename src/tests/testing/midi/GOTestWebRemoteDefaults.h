/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTWEBREMOTEDEFAULTS_H
#define GOTESTWEBREMOTEDEFAULTS_H

#include "GOTest.h"

// Checks that a fresh config wires the Web Remote page's default buttons to
// the Combination Setter, and leaves the other setter buttons alone.
class GOTestWebRemoteDefaults : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestSetterButtonsAreMapped();
  void TestOtherButtonsAreNot();
  void TestKeepsMappingsFromOtherDevices();
  void TestAddMissingEventsFrom();

public:
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  bool tearDown() override;
  void run() override;
};

#endif /* GOTESTWEBREMOTEDEFAULTS_H */
