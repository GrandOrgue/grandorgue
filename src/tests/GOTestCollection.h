/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTCOLLECTION_H
#define GOTESTCOLLECTION_H

#include "GOTest.h"
#include "GOTestResultCollection.h"
#include <vector>
#include <wx/string.h>

class GOTestCollection {
  /*
      This is the class that collects all the tests (derived from GOTest)
  */

private:
  // As the test collection should be singleton, its pointer.
  static GOTestCollection *go_test_collection;
  // Private constructor
  GOTestCollection();
  std::vector<GOTest *> tests_;
  int run_number_ = 0;
  int fail_count_ = 0;
  int success_count_ = 0;

public:
  static GOTestCollection *Instance() {
    //
    // instanciation unique garantie, du moins si
    // le programme n'a qu'un seul fil d'exécution;
    // si votre programme comprend plusieurs fils
    // d'exécution, ce qui suit est insuffisant
    //
    if (!go_test_collection)
      go_test_collection = new GOTestCollection;
    return go_test_collection;
  }
  //   GOTestCollection();
  //   virtual ~GOTestCollection();
  GOTestResultCollection run();
  void add_test(GOTest *test);
  int get_failed_count();
  int get_success_count();
};

#endif
