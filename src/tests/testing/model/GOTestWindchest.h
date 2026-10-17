/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
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

  /*
   * Windchest numbering, name, amplitude from an enclosure, tremulants, ranks
   * and pipes.
   */
  void TestWindchestBasics();

public:
  GOTestWindchest() { name = "GOTestWindchest"; }
  virtual ~GOTestWindchest();
  virtual void run();
  std::string GetName();
};

#endif
