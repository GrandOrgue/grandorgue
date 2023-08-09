/*
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEST_H
#define GOTEST_H

#include <wx/string.h>

class GOTest {
  /*
      We define here all methods that will be called to run tests properly.
          - A setup
          - The run itself
          - A tear down method
  */

public:
  GOTest();
  virtual ~GOTest();
  virtual bool setUp();
  virtual void run();
  virtual bool tearDown();

  wxString name = "GOTest";
};

#endif