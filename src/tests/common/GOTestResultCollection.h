/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTRESULTCOLLECTION_H
#define GOTESTRESULTCOLLECTION_H

#include "GOTest.h"
#include "GOTestResult.h"
#include <vector>

class GOTestResultCollection {

private:
  std::vector<GOTestResult *> test_results;

public:
  GOTestResultCollection();
  void add_result(GOTestResult *test);
  std::vector<GOTestResult *> get_results();
};

#endif