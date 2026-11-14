/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTMIDISYSTEM_H
#define GOTESTMIDISYSTEM_H

#include "GOTest.h"

class GOTestMidiSystem : public GOTest {
private:
  void TestLateDiscovery();
  void TestFailedOpen();

public:
  std::string GetName() override { return "GOTestMidiSystem"; }
  void run() override;
};

#endif /* GOTESTMIDISYSTEM_H */
