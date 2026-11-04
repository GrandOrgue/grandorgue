/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <vector>

#include "common/GOTestCollection.h"
#include "common/GOTestFilter.h"
#include "common/GOTestScope.h"
#include "testing/GOTestNameMap.h"
#include "testing/GOTestOrganController.h"
#include "testing/combinations/GOTestDivisionalSetter.h"
#include "testing/loader/GOTestOrganReader.h"
#include "testing/midi/GOTestMidiEvent.h"
#include "testing/midi/GOTestMidiPlayerContent.h"
#include "testing/midi/GOTestMidiSendProxy.h"
#include "testing/model/GOTestDrawStop.h"
#include "testing/model/GOTestEnclosure.h"
#include "testing/model/GOTestOrganModel.h"
#include "testing/model/GOTestPipeConfigTreeNode.h"
#include "testing/model/GOTestSoundingPipe.h"
#include "testing/model/GOTestSwitch.h"
#include "testing/model/GOTestWindchest.h"
#include "testing/scheduler/GOTestScheduler.h"
#include "testing/sound/GOTestSoundCallbackConnector.h"
#include "testing/sound/GOTestSoundOrganEngine.h"
#include "testing/sound/GOTestSoundOrganEngineFactories.h"
#include "testing/sound/GOTestSoundOrganEngineStress.h"
#include "testing/sound/GOTestSoundWindchestGroupTaskGrid.h"
#include "testing/sound/buffer/GOTestPerfSoundBufferMutable.h"
#include "testing/sound/buffer/GOTestPerfSoundBufferPlanarMutable.h"
#include "testing/sound/buffer/GOTestSoundBuffer.h"
#include "testing/sound/buffer/GOTestSoundBufferManaged.h"
#include "testing/sound/buffer/GOTestSoundBufferMono.h"
#include "testing/sound/buffer/GOTestSoundBufferMutable.h"
#include "testing/sound/buffer/GOTestSoundBufferMutableMono.h"
#include "testing/sound/buffer/GOTestSoundBufferPlanar.h"
#include "testing/sound/buffer/GOTestSoundBufferPlanarManaged.h"
#include "testing/sound/buffer/GOTestSoundBufferPlanarMutable.h"
#include "testing/sound/buffer/GOTestSoundTypedBufferLoopBase.h"
#include "testing/sound/buffer/GOTestSoundTypedBufferLoopMonoManaged.h"
#include "testing/sound/dsp-kernels/GOTestPerfSoundLinearFaderCore.h"
#include "testing/sound/dsp-kernels/GOTestPerfSoundResample.h"
#include "testing/sound/dsp-kernels/GOTestSoundLinearFaderCore.h"
#include "testing/sound/dsp-kernels/GOTestSoundOnePoleFilter.h"
#include "testing/sound/dsp-kernels/GOTestSoundResample.h"
#include "testing/sound/effects/GOTestPerfSoundShelfFilterProcessor.h"
#include "testing/sound/effects/GOTestPerfSoundVibratoPitchRateCurve.h"
#include "testing/sound/effects/GOTestPerfSoundVibratoProcessor.h"
#include "testing/sound/effects/GOTestSoundReverbProcessor.h"
#include "testing/sound/effects/GOTestSoundShelfFilterProcessor.h"
#include "testing/sound/effects/GOTestSoundVibratoPitchRateCurve.h"
#include "testing/sound/effects/GOTestSoundVibratoProcessor.h"
#include "testing/sound/mappers/GOTestPerfSoundEnclosureShelfMapper.h"
#include "testing/sound/mappers/GOTestSoundEnclosureShelfMapper.h"
#include "testing/sound/playing/GOTestPerfSoundPlayingFader.h"
#include "testing/sound/playing/GOTestReleaseAlignTable.h"
#include "testing/sound/playing/GOTestSoundPlayingFader.h"
#include "testing/sound/playing/GOTestSoundStream.h"
#include "testing/sound/playing/GOTestSoundToneBalanceFilter.h"
#include "testing/sound/processing/GOTestSoundProcessingChain.h"
#include "testing/sound/processing/GOTestSoundProcessorTyped.h"
#include "testing/sound/providers/GOTestSoundTremulantProviderSynth.h"
#include "testing/sound/reverb/GOTestSoundReverb.h"
#include "testing/sound/tasks/GOTestPerfSoundTaskBase.h"
#include "testing/sound/tasks/GOTestSoundOutputTask.h"
#include "testing/sound/tasks/GOTestSoundRecorderTask.h"
#include "testing/sound/tasks/GOTestSoundTaskBase.h"
#include "testing/sound/tasks/GOTestSoundWindchestGroupTask.h"
#include "testing/sound/tasks/GOTestSoundWindchestTask.h"

enum ExitCode {
  OK = 0,
  NOTHING_SELECTED = 1,
  TESTS_FAILED = 2,
  BAD_ARGUMENTS = 3
};

static void print_usage() {
  std::cout
    << "Usage: GOTestExe [options] [pattern...]\n"
       "Options:\n"
       "  --perf-only        run only performance tests\n"
       "  --no-perf          run only functional tests\n"
       "  --list             list the selected test classes and tests, do not "
       "run them\n"
       "  --timing-top N     show N slowest tests in the timing summary "
       "(default 20, 0 = all)\n"
       "  --help             show this help\n"
       "Pattern: [!]ClassRegex[/TestRegex], several patterns may be given as "
       "separate arguments.\n"
       "  A regular expression is searched in the class or test name. A "
       "pattern without '/' selects whole classes.\n"
       "  A pattern starting with '!' excludes. Quote patterns in the "
       "shell.\n"
       "Exit codes: 0 - ok, 1 - nothing selected, 2 - some tests failed, 3 - "
       "wrong arguments\n";
}

/**
 * Parses the unsigned value of an option. Only decimal digits are accepted:
 * strtoul() would also accept leading spaces and a sign ('-1' would become
 * ULONG_MAX) and saturates on overflow. The reason is printed if the value is
 * not valid.
 * @param option the name of the option, for the error message
 * @param value the next command line argument or nullptr if there is no one
 * @param outNumber the parsed number
 * @return OK or BAD_ARGUMENTS
 */
static ExitCode parse_unsigned(
  const char *option, const char *value, unsigned &outNumber) {
  ExitCode result = BAD_ARGUMENTS;

  if (!value)
    std::cerr << "The option " << option << " needs a value\n";
  else if (!std::isdigit(static_cast<unsigned char>(*value)))
    std::cerr << "The value '" << value << "' of " << option
              << " must be a number consisting of decimal digits only\n";
  else {
    char *pEnd = nullptr;

    errno = 0;

    const unsigned long number = std::strtoul(value, &pEnd, 10);

    if (*pEnd)
      std::cerr << "The value '" << value << "' of " << option
                << " must be a number consisting of decimal digits only\n";
    else if (errno == ERANGE || number > UINT_MAX)
      std::cerr << "The value '" << value << "' of " << option
                << " is too large: the maximum is " << UINT_MAX << "\n";
    else {
      outNumber = static_cast<unsigned>(number);
      result = OK;
    }
  }
  return result;
}

/**
 * Adds the pattern given in a command line argument
 * @return OK or BAD_ARGUMENTS if a regular expression is not valid
 */
static ExitCode add_pattern(const std::string &pattern) {
  ExitCode result = OK;

  try {
    GOTestFilter::getInstance().AddPattern(pattern);
  } catch (const std::regex_error &e) {
    std::cerr << "Invalid pattern '" << pattern << "': " << e.what()
              << std::endl;
    result = BAD_ARGUMENTS;
  }
  return result;
}

/**
 * Prints the two timing tables: all test classes and the slowest tests
 * @param nTop the number of the slowest tests to show, 0 - all
 */
static void print_timing(unsigned nTop) {
  std::vector<GOTestScope::Record> classes;
  std::vector<GOTestScope::Record> tests;
  long totalMs = 0;

  for (const GOTestScope::Record &record : GOTestScope::getRecords()) {
    if (record.m_kind == GOTestScope::TEST_CLASS) {
      classes.push_back(record);
      totalMs += record.m_ElapsedMs;
    } else
      tests.push_back(record);
  }

  const auto isSlower
    = [](const GOTestScope::Record &a, const GOTestScope::Record &b) {
        return a.m_ElapsedMs > b.m_ElapsedMs;
      };

  std::stable_sort(classes.begin(), classes.end(), isSlower);
  std::stable_sort(tests.begin(), tests.end(), isSlower);

  const unsigned nShownTests
    = nTop && nTop < tests.size() ? nTop : (unsigned)tests.size();

  std::cout << "==================== TESTS TIMING =====================\n";
  std::cout << "Test classes (total " << totalMs << " ms):\n";
  for (const GOTestScope::Record &record : classes)
    std::cout << std::setw(10) << record.m_ElapsedMs << " ms  " << record.m_path
              << "\n";
  std::cout << "Slowest tests (" << nShownTests << " of " << tests.size()
            << "):\n";
  for (unsigned testI = 0; testI < nShownTests; ++testI)
    std::cout << std::setw(10) << tests[testI].m_ElapsedMs << " ms  "
              << tests[testI].m_path << "\n";
  std::cout << "=======================================================\n";
}

int main(int argc, char *argv[]) {
  /*
      This is the main function that will collect all tests in the collection,
      then run the whole bunch. See print_usage() for the supported arguments.
  */

  std::optional<GOTest::Category> categoryFilter;
  ExitCode exitCode = OK;
  bool isRunNeeded = true;
  unsigned nTop = 20;

  for (int argI = 1; argI < argc && exitCode == OK && isRunNeeded; ++argI) {
    const std::string arg = argv[argI];

    if (arg == "--perf-only")
      categoryFilter = GOTest::PERF;
    else if (arg == "--no-perf")
      categoryFilter = GOTest::FUNCTIONAL;
    else if (arg == "--list")
      GOTestFilter::getInstance().SetListOnly(true);
    else if (arg == "--timing-top") {
      argI++;
      exitCode = parse_unsigned(
        "--timing-top", argI < argc ? argv[argI] : nullptr, nTop);
    } else if (arg == "--help")
      isRunNeeded = false;
    else if (arg[0] == '-')
      exitCode = BAD_ARGUMENTS; // an unknown option
    else
      exitCode = add_pattern(arg);
  }
  if (exitCode != OK || !isRunNeeded)
    print_usage();

  /* Instantiate all the test classes here */
  GOTestDivisionalSetter testDivisionalSetter;
  GOTestOrganReader testOrganReader;
  GOTestOrganController testOrganController;
  GOTestDrawStop testDrawStop;
  GOTestEnclosure testEnclosure;
  GOTestOrganModel testOrganModel;
  GOTestPipeConfigTreeNode testPipeConfigTreeNode;
  GOTestSoundingPipe testSoundingPipe;
  GOTestSwitch testSwitch;
  GOTestWindchest testWindchest;
  GOTestScheduler testScheduler;
  GOTestNameMap goTestNameMap;
  GOTestMidiSendProxy testMidiSendProxy;
  GOTestMidiEvent testMidiEvent;
  GOTestMidiPlayerContent testMidiPlayerContent;
  GOTestSoundBuffer goTestSoundBuffer;
  GOTestSoundBufferManaged testSoundBufferManaged;
  GOTestSoundBufferMono testSoundBufferMono;
  GOTestSoundBufferMutable testSoundBufferMutable;
  GOTestSoundBufferMutableMono testSoundBufferMutableMono;
  GOTestSoundBufferPlanar testSoundBufferPlanar;
  GOTestSoundBufferPlanarMutable testSoundBufferPlanarMutable;
  GOTestSoundBufferPlanarManaged testSoundBufferPlanarManaged;
  GOTestSoundTypedBufferLoopBase testSoundTypedBufferLoopBase;
  GOTestSoundTypedBufferLoopMonoManaged testSoundTypedBufferLoopMonoManaged;
  GOTestPerfSoundBufferMutable testPerfSoundBufferMutable;
  GOTestPerfSoundBufferPlanarMutable testPerfSoundBufferPlanarMutable;
  GOTestSoundProcessingChain testSoundProcessingChain;
  GOTestSoundProcessorTyped testSoundProcessorTyped;
  GOTestSoundOrganEngine testSoundOrganEngine;
  GOTestSoundOrganEngineFactories testSoundOrganEngineFactories;
  GOTestSoundCallbackConnector testSoundCallbackConnector;
  GOTestSoundOrganEngineStress testSoundOrganEngineStress;
  GOTestReleaseAlignTable testReleaseAlignTable;
  GOTestSoundPlayingFader testSoundPlayingFader;
  GOTestPerfSoundPlayingFader testPerfSoundPlayingFader;
  GOTestSoundStream testSoundStream;
  GOTestSoundLinearFaderCore testSoundLinearFaderCore;
  GOTestPerfSoundLinearFaderCore testPerfSoundLinearFaderCore;
  GOTestSoundOnePoleFilter testSoundOnePoleFilter;
  GOTestSoundResample testSoundResample;
  GOTestPerfSoundResample testPerfSoundResample;
  GOTestSoundToneBalanceFilter testSoundToneBalanceFilter;
  GOTestSoundTremulantProviderSynth testSoundTremulantProviderSynth;
  GOTestSoundReverb testSoundReverb;
  GOTestSoundReverbProcessor testSoundReverbProcessor;
  GOTestSoundShelfFilterProcessor testSoundShelfFilterProcessor;
  GOTestPerfSoundShelfFilterProcessor testPerfSoundShelfFilterProcessor;
  GOTestSoundVibratoPitchRateCurve testSoundVibratoPitchRateCurve;
  GOTestSoundVibratoProcessor testSoundVibratoProcessor;
  GOTestPerfSoundVibratoPitchRateCurve testPerfSoundVibratoPitchRateCurve;
  GOTestPerfSoundVibratoProcessor testPerfSoundVibratoProcessor;
  GOTestSoundEnclosureShelfMapper testSoundEnclosureShelfMapper;
  GOTestPerfSoundEnclosureShelfMapper testPerfSoundEnclosureShelfMapper;
  GOTestSoundWindchestGroupTaskGrid testSoundWindchestGroupTaskGrid;
  GOTestSoundTaskBase testSoundTaskBase;
  GOTestSoundWindchestTask testSoundWindchestTask;
  GOTestSoundOutputTask testSoundOutputTask;
  GOTestSoundRecorderTask testSoundRecorderTask;
  GOTestSoundWindchestGroupTask testSoundWindchestGroupTask;
  GOTestPerfSoundTaskBase testPerfSoundTaskBase;
  /* end of instanciation */

  if (exitCode == OK && isRunNeeded) {
    GOTestResultCollection test_result_collection;

    test_result_collection = GOTestCollection::Instance()->Run(categoryFilter);
    if (GOTestFilter::getInstance().GetNSelected() == 0) {
      std::cerr << "No tests matched the given patterns" << std::endl;
      exitCode = NOTHING_SELECTED;
    } else if (!GOTestFilter::getInstance().IsListOnly()) {
      // Display tests results
      std::cout << "==================== TESTS RESULTS ====================\n";
      for (GOTestResult &result : test_result_collection.get_results()) {
        std::cout
          << "-------------------------------------------------------\n";
        std::cout << result.GetMessage() << "\n";
      }

      const int failed_count = GOTestCollection::Instance()->get_failed_count();
      const int success_count
        = GOTestCollection::Instance()->get_success_count();

      std::cout << "==================== TESTS SUMMARY ====================\n";
      if (failed_count > 0) {
        std::cout << failed_count << " tests Failed\n";
        exitCode = TESTS_FAILED;
      } else {
        std::cout << success_count << " tests Succeeded\n";
        std::cout
          << "=======================================================\n";
      }
      print_timing(nTop);
    }
  }
  return exitCode;
}
