/*
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEST_H
#define GOTEST_H

#include "GOOrganController.h"
#include <wx/string.h>

class GOTest {
  /*
      We define here all methods that will be called to run tests properly.
          - A setup
          - The run itself
          - A tear down method
  */

private:
  wxString name = "GOTest";

public:
  GOTest();
  virtual ~GOTest();
  virtual bool setUp();
  virtual void run();
  virtual bool tearDown();
  virtual wxString GetName() { return name; };
};

class GOCommonControllerTest : public GOTest {

public:
  GOCommonControllerTest();
  char *organ_directory;
  GOOrganController *controller;
  virtual bool setUp();
  virtual bool tearDown();
};

#endif