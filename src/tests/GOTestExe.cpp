/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestCollection.h"
#include "GOTestOrganModel.h"
#include "GOTestSwitch.h"
#include "GOTestWindchest.h"
#include <cstdio>
#include <iostream>

int main() {
  /*
      This is the main function that will collect all tests in the collection,
      then run the whole bunch.

      TODO: It should displays also the tests results
  */

  /* Instantiate all the test classes here */
  new GOTestWindchest;
  new GOTestOrganModel;
  new GOTestSwitch;
  /* end of instanciation */
  GOTestResultCollection test_result_collection;
  test_result_collection = GOTestCollection::Instance()->run();

  // Display tests results
  int run_number_ = 0;
  std::vector<GOTestResult *> test_results
    = test_result_collection.get_results();
  std::cout << "==================== TESTS RESULTS ====================\n";
  for (auto current = test_results.begin(); current != test_results.end();
       ++current, ++run_number_) {
    auto test_result = *current;
    std::cout << "-------------------------------------------------------\n";
    std::cout << test_result->GetMessage() << "\n";
  }

  const int failed_count = GOTestCollection::Instance()->get_failed_count();
  const int success_count = GOTestCollection::Instance()->get_success_count();
  std::cout << "==================== TESTS SUMMARY ====================\n";
  if (failed_count > 0) {
    std::cout << failed_count << " tests Failed\n";
    return 1;
  }
  std::cout << success_count << " tests Succeeded\n";
  std::cout << "=======================================================\n";
  return 0;
}
