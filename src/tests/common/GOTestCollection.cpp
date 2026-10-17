/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestCollection.h"
#include "GOTestException.h"
#include "GOTestFilter.h"
#include "GOTestResultCollection.h"
#include "GOTestScope.h"
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

bool GOTestCollection::HasSelectedTests(GOTest *test) {
  GOTestScope::setCurrentClass(test->GetName());
  GOTestScope::setProbing(true);
  try {
    test->run();
  } catch (...) {
    // a failure will be reported by the real run
  }
  GOTestScope::setProbing(false);
  return !GOTestScope::hasNoSelectedTests();
}

GOTestResultCollection GOTestCollection::Run(
  std::optional<GOTest::Category> categoryFilter) {
  /*
      This iterates on tests_ vector, run them one by one, then collects
      the tests results and display them at the end.
  */
  GOTestResultCollection test_result_collection;

  run_number_ = 0;
  for (auto current = tests_.begin(); current != tests_.end();
       ++current, ++run_number_) {
    auto test = *current;

    if (
      (!categoryFilter.has_value() || test->GetCategory() == *categoryFilter)
      && GOTestFilter::getInstance().IsClassSelected(test->GetName())
      && HasSelectedTests(test)) {
      if (GOTestFilter::getInstance().IsListOnly()) {
        // No GOTestScope, setUp() and tearDown(): GO_RUN_TEST only prints the
        // names, so the output can be copied to the patterns
        std::cout << test->GetName() << std::endl;
        GOTestScope::setCurrentClass(test->GetName());
        test->run();
      } else {
        RunTest(test, test_result_collection);
      }
    }
  }
  return test_result_collection;
}

void GOTestCollection::RunTest(
  GOTest *test, GOTestResultCollection &test_result_collection) {
  GOTestScope testClassScope(GOTestScope::TEST_CLASS, test->GetName());

  try {
    if (test->setUp()) {
      bool isRunSucceeded = false;

      try {
        test->run();
        isRunSucceeded = true;
      } catch (GOTestException &e) {
        fail_count_++;
        test_result_collection.add_result(
          GOTestResult(test->GetName() + " failed: " + e.what(), true));
        test->tearDown();
      } catch (std::exception &e) {
        fail_count_++;
        test_result_collection.add_result(
          GOTestResult(test->GetName() + " failed: " + e.what(), true));
        test->tearDown();
      }
      if (isRunSucceeded) {
        test_result_collection.add_result(
          GOTestResult(test->GetName() + " succeeded"));
        success_count_++;
        test->tearDown();
      }
    } else {
      test_result_collection.add_result(GOTestResult(
        "The setUp() of test '" + test->GetName() + "' has failed."));
    }
  } catch (std::exception &e) {
    test_result_collection.add_result(GOTestResult(
      "An exception occurred during test '" + test->GetName() + "'."));
  } catch (...) {
    fail_count_++;
    test_result_collection.add_result(GOTestResult("Unknown exception", true));
    test->tearDown();
  }
}

GOTestCollection *GOTestCollection::go_test_collection = nullptr;
