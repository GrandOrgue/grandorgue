/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include "GOTestCollection.h"
#include "GOTestResultCollection.h"
#include <iostream>
#include <iterator>
#include <vector>

GOTestCollection::GOTestCollection() {}

// GOTestCollection::~GOTestCollection() {
//   // Destroy the initialized tests in vector
//   tests_.clear();
// }

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
    try {
      auto test = *current;
      if (test->setUp()) {
        try {
          test->run();
          test_result_collection->add_result(
            new GOTestResult(test->GetName() << " succeeded"));
          success_count_++;
        } catch (std::exception &e) {
          fail_count_++;
          test_result_collection->add_result(new GOTestResult(e.what(), true));
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
  return *test_result_collection;
}

GOTestCollection *GOTestCollection::go_test_collection = nullptr;

// class X {
//    // pointeur vers le singleton
//    static X *singleton;
//    X(); // constructeur privé
// public:
//    // pour obtenir l'singleton
//    static X *get()    {
//       //
//       // instanciation unique garantie, du moins si
//       // le programme n'a qu'un seul fil d'exécution;
//       // si votre programme comprend plusieurs fils
//       // d'exécution, ce qui suit est insuffisant
//       //
//       if (!singleton)
//          singleton = new X;
//       return singleton;
//    }
//    // ...
// };
// // initialisation du pointeur de singleton à zéro
// // (insérer dans le .cpp)
// X *X::singleton = nullptr;