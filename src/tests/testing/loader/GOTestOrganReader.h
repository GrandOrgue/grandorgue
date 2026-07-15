/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTORGANREADER_H
#define GOTESTORGANREADER_H

#include <string>

#include "GOTest.h"

class GOTestOrganReader : public GOCommonControllerTest {

private:
  std::string name = "GOTestOrganReader";

public:
  GOTestOrganReader() { name = "GOTestOrganReader"; }
  virtual ~GOTestOrganReader();
  virtual void run();
  std::string GetName();

private:
  void runImpl();
};

#endif
