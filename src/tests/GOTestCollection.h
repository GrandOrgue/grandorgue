/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTCOLLECTION_H
#define GOTESTCOLLECTION_H

#include "GOTest.h"
#include <vector>
#include <wx/string.h>

class GOTestCollection {
  /*
      This is the class that collects all the tests (derived from GOTest)
  */
private:
  std::vector<GOTest *> tests_;
  int run_number_ = 0;
  int fail_count_ = 0;
  int success_count_ = 0;

public:
  GOTestCollection();
  virtual ~GOTestCollection();
  void run();
  void add_test(GOTest *test);
  int get_failed_count();
  int get_success_count();
};

#endif
