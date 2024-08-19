#ifndef GOTESTUTILS_H
#define GOTESTUTILS_H

#include "GOTestException.h"
#include <cpptrace.hpp>

class GOTestUtils {
  /*
      We define here all methods that should be used in tests to check
      values and raise a GOTestException
  */

public:
  void GOAssert(bool expression, std::string message) {
    /**
     * This util function should be called as soon as we want to test an
     * expression in the test framework and launch the appropriate Exception
     * in GrandOrgue context with a good stack trace that shows us the
     * culprit line in order to find it easily.
     *
     */
    if (expression != true) {
      // The stack trace should be put here as all tests inherits
      // from this util class in order to display the original
      // faulty line in stack.
      std::cout << cpptrace::generate_trace();
      throw GOTestException(message);
    }
  }
};
#endif
