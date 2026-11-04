/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTMIDIWEBINPORT_H
#define GOTESTMIDIWEBINPORT_H

#include "GOTest.h"

// What the Web Remote http server lets through to the MIDI system
class GOTestMidiWebInPort : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestAcceptsChannelMessages();
  void TestRejectsEverythingElse();
  void TestServesAndClosesFast();

public:
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  bool tearDown() override;
  void run() override;
};

#endif /* GOTESTMIDIWEBINPORT_H */
