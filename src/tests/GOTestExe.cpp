/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <iostream>

#include "GOTestCollection.h"
#include "GOTestWindchest.h"

int main() {
  /*
      This is the main function that will collect all tests in the collection,
      then run the whole bunch.

      TODO: It should displays also the tests results
  */

  GOTestCollection go_collection;
  go_collection.add_test(new GOTestWindchest());
  go_collection.run();
  const int failed_count = go_collection.get_failed_count();
  const int success_count = go_collection.get_success_count();
  if (failed_count > 0) {
    std::cerr << failed_count << " tests Failed";
    std::cout << success_count << " tests Succeeded";
    return 1;
  }
  std::cout << success_count << " tests Succeeded";
  return 0;
}
