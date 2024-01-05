/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include "GOTestResultCollection.h"
#include <iostream>
#include <iterator>
#include <vector>

GOTestResultCollection::GOTestResultCollection() {}

void GOTestResultCollection::add_result(GOTestResult *test_result) {
  /*
      This method allows to add tests results.
  */
  this->test_results.push_back(test_result);
}

std::vector<GOTestResult *> GOTestResultCollection::get_results() {

  return this->test_results;
}
