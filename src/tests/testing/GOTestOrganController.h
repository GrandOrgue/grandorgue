/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTORGANCONTROLLER_H
#define GOTESTORGANCONTROLLER_H

#include <string>

#include "GOTest.h"

class GOTestOrganController : public GOCommonControllerTest {

private:
  std::string name = "GOTestOrganController";

public:
  GOTestOrganController() { name = "GOTestOrganController"; }
  virtual ~GOTestOrganController();
  virtual void run();
  std::string GetName();

private:
  void runImpl();
};

#endif /* GOTESTORGANCONTROLLER_H */
