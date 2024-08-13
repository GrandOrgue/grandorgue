/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTWINDCHEST_H
#define GOTESTWINDCHEST_H

#include "GOTest.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"

class GOTestWindchest : public GOCommonControllerTest {

private:
  std::string name = "Windchest";

public:
  GOTestWindchest() { name = "GOTestWindchest"; }
  virtual ~GOTestWindchest();
  virtual void run();
  std::string GetName();
};

#endif
