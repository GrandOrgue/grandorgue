/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include "GOTestCollection.h"
#include <iostream>
#include <iterator>
#include <vector>

GOTestCollection::GOTestCollection() {}

GOTestCollection::~GOTestCollection() {
  // Destroy the initialized tests in vector
  tests_.clear();
}

void GOTestCollection::add_test(GOTest *test) {
  /*
      This method allows to add tests classes (derived from GOTest).
  */
  this->tests_.push_back(test);
}

int GOTestCollection::get_failed_count() {
  /*
      This method returns the amount of tests that failed.
  */
  return fail_count_;
}

int GOTestCollection::get_success_count() {
  /*
      This method returns the amount of tests that succeeded.
  */
  return success_count_;
}

void GOTestCollection::run() {
  /*
      This iterates on tests_ vector, run them one by one, then collects
      the tests results and display them at the end.
  */
  run_number_ = 0;
  for (auto current = tests_.begin(); current != tests_.end();
       ++current, ++run_number_) {
    try {
      auto test = *current;
      if (test->setUp()) {
        try {
          test->run();
          success_count_++;
        } catch (...) {
          fail_count_++;
          test->tearDown();
          continue;
        }
        if (!test->tearDown()) {
          continue;
        }
      } else {
        // error("Setup failed");
      }
    } catch (std::exception &e) {
      // error(e.what());
    } catch (...) {
      // error("Unexpected error");
    }
  }
}