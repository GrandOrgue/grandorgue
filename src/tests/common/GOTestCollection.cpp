/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestCollection.h"
#include "GOTestException.h"
#include "GOTestResultCollection.h"
#include <iostream>
#include <iterator>
#include <vector>

GOTestCollection::GOTestCollection() {}

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

GOTestResultCollection GOTestCollection::run() {
  /*
      This iterates on tests_ vector, run them one by one, then collects
      the tests results and display them at the end.
  */
  GOTestResultCollection *test_result_collection = new GOTestResultCollection();
  run_number_ = 0;
  for (auto current = tests_.begin(); current != tests_.end();
       ++current, ++run_number_) {
    auto test = *current;
    try {
      if (test->setUp()) {
        try {
          test->run();
          test_result_collection->add_result(
            new GOTestResult(test->GetName() + " succeeded"));
          success_count_++;
        } catch (GOTestException &e) {
          fail_count_++;
          test_result_collection->add_result(
            new GOTestResult(test->GetName() + " failed: " + e.what(), true));
          test->tearDown();
          continue;
        } catch (std::exception &e) {
          fail_count_++;
          test_result_collection->add_result(
            new GOTestResult(test->GetName() + " failed: " + e.what(), true));
          test->tearDown();
          continue;
        }
        if (!test->tearDown()) {
          continue;
        }
      } else {
        test_result_collection->add_result(new GOTestResult(
          "The setUp() of test '" + test->GetName() + "' has failed."));
      }
    } catch (std::exception &e) {
      test_result_collection->add_result(new GOTestResult(
        "An exception occurred during test '" + test->GetName() + "'."));
    } catch (...) {
      fail_count_++;
      test_result_collection->add_result(
        new GOTestResult("Unknown exception", true));
      test->tearDown();
      continue;
    }
  }
  return *test_result_collection;
}

GOTestCollection *GOTestCollection::go_test_collection = nullptr;
