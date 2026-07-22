/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <cstdio>
#include <iostream>
#include <optional>
#include <string>

#include "common/GOTestCollection.h"
#include "testing/GOTestNameMap.h"
#include "testing/midi/GOTestMidiPlayerContent.h"
#include "testing/model/GOTestDrawStop.h"
#include "testing/model/GOTestOrganModel.h"
#include "testing/model/GOTestSwitch.h"
#include "testing/model/GOTestWindchest.h"
#include "testing/sound/buffer/GOTestPerfSoundBufferMutable.h"
#include "testing/sound/buffer/GOTestSoundBuffer.h"
#include "testing/sound/buffer/GOTestSoundBufferManaged.h"
#include "testing/sound/buffer/GOTestSoundBufferMutable.h"
#include "testing/sound/buffer/GOTestSoundBufferMutableMono.h"
#include "testing/sound/playing/GOTestReleaseAlignTable.h"
#include "testing/sound/playing/GOTestSoundStream.h"

int main(int argc, char *argv[]) {
  /*
      This is the main function that will collect all tests in the collection,
      then run the whole bunch.

      Supported arguments:
        --perf-only   run only performance tests (GOTest::PERF)
        --no-perf     run only functional tests (GOTest::FUNCTIONAL)
        (no argument) run all tests
  */

  std::optional<GOTest::Category> categoryFilter;

  for (int argI = 1; argI < argc; ++argI) {
    const std::string arg = argv[argI];

    if (arg == "--perf-only")
      categoryFilter = GOTest::PERF;
    else if (arg == "--no-perf")
      categoryFilter = GOTest::FUNCTIONAL;
  }

  /* Instantiate all the test classes here */
  GOTestDrawStop testDrawStop;
  GOTestOrganModel testOrganModel;
  GOTestSwitch testSwitch;
  GOTestWindchest testWindchest;
  GOTestNameMap goTestNameMap;
  GOTestMidiPlayerContent testMidiPlayerContent;
  GOTestSoundBuffer goTestSoundBuffer;
  GOTestSoundBufferManaged testSoundBufferManaged;
  GOTestSoundBufferMutable testSoundBufferMutable;
  GOTestSoundBufferMutableMono testSoundBufferMutableMono;
  GOTestPerfSoundBufferMutable testPerfSoundBufferMutable;
  GOTestReleaseAlignTable testReleaseAlignTable;
  GOTestSoundStream testSoundStream;
  /* end of instanciation */
  GOTestResultCollection test_result_collection;
  test_result_collection = GOTestCollection::Instance()->Run(categoryFilter);

  // Display tests results
  std::cout << "==================== TESTS RESULTS ====================\n";
  for (GOTestResult *pResult : test_result_collection.get_results()) {
    std::cout << "-------------------------------------------------------\n";
    std::cout << pResult->GetMessage() << "\n";
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
