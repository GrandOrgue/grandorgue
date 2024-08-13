/*
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEST_H
#define GOTEST_H

#include "GOOrganController.h"
#include "GOTestUtils.h"

class GOTest : public GOTestUtils {
  /*
      We define here all methods that will be called to run tests properly.
          - A setup
          - The run itself
          - A tear down method
  */

private:
  std::string name = "GOTest";

public:
  GOTest();
  virtual ~GOTest();
  virtual bool setUp();
  virtual void run();
  virtual bool tearDown();
  virtual std::string GetName() { return name; };
};

class GOCommonControllerTest : public GOTest {

public:
  GOCommonControllerTest();
  char *organ_directory;
  GOOrganController *controller;
  bool setUp();
  virtual bool tearDown();
};

#endif