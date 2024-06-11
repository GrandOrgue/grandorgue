/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSWITCH_H
#define GOTESTSWITCH_H

#include <iostream>

#include "GOTest.h"

class GOTestSwitch : public GOCommonControllerTest {

private:
  std::string name = "GOTestSwitch";

public:
  GOTestSwitch() { name = "GOTestSwitch"; }
  virtual ~GOTestSwitch();
  virtual void run();
  std::string GetName();
};

#endif