/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundEnclosureShelfMapper.h"

#include <chrono>
#include <format>
#include <iostream>

#include "config/GOConfig.h"
#include "model/GOEnclosure.h"
#include "model/GOOrganModel.h"
#include "sound/GOSoundDefs.h"
#include "sound/effects/GOSoundShelfFilterProcessor.h"
#include "sound/mappers/GOSoundEnclosureShelfMapper.h"

#include "GOTestScope.h"

const std::string GOTestPerfSoundEnclosureShelfMapper::TEST_NAME
  = "GOTestPerfSoundEnclosureShelfMapper";

static constexpr unsigned SAMPLE_RATE = 96000;
static constexpr unsigned N_FRAMES = 32;
static constexpr long N_ITERATIONS = 2'000'000;

static constexpr uint8_t BASELINE_MIDI = 64;
static constexpr float BASELINE_LOW_FREQUENCY = 200.0f;
static constexpr float BASELINE_LOW_ATTENUATION_DB = 12.0f;
static constexpr float BASELINE_HIGH_FREQUENCY = 5000.0f;
static constexpr float BASELINE_HIGH_ATTENUATION_DB = 6.0f;
static constexpr float TOGGLED_LOW_FREQUENCY = BASELINE_LOW_FREQUENCY + 100;
static constexpr float TOGGLED_HIGH_FREQUENCY = BASELINE_HIGH_FREQUENCY + 1000;

// Minimum acceptable throughput of the steady-state round: neither band's
// config nor the enclosure's position changed, so the mapper takes only the
// cheap comparison path with no computeCoeffs() call at all. Thresholds are
// set at roughly 20% of the minimum observed on a local Debug run
// (2026-09-01, in isolation: 183.8M/22.6M/12.5M calls/sec for the three
// cases below) - a wider margin than a simple halving, matching
// GOTestPerfSoundShelfFilterProcessor.cpp's BASELINE_BOTH_NOOP margin
// because the "nothing changed" case here is likewise dominated by fixed
// per-call overhead and sensitive to contention from the rest of the test
// suite running at the same time. No separate Release numbers are available
// yet, so the Release thresholds reuse the same Debug-derived floor rather
// than guessing a higher one.
// TODO: recalibrate once this test has run on CI, separately for each build
// type - same precedent as GOTestPerfSoundTaskBase.cpp's
// MIN_COOPERATIVE_ITEMS_PER_SECOND.
#ifdef NDEBUG
static constexpr double MIN_NOTHING_CHANGED_CALLS_PER_SECOND
  = 36'000'000.0; // 36.0M calls/sec (20% of isolated min observed 183.8M)
static constexpr double MIN_ONLY_HIGH_CHANGED_CALLS_PER_SECOND
  = 4'500'000.0; // 4.5M calls/sec (20% of isolated min observed 22.6M)
static constexpr double MIN_BOTH_CHANGED_CALLS_PER_SECOND
  = 2'500'000.0; // 2.5M calls/sec (20% of isolated min observed 12.5M)
#else
static constexpr double MIN_NOTHING_CHANGED_CALLS_PER_SECOND
  = 36'000'000.0; // 36.0M calls/sec (debug, 20% of isolated min observed
                  // 183.8M)
static constexpr double MIN_ONLY_HIGH_CHANGED_CALLS_PER_SECOND
  = 4'500'000.0; // 4.5M calls/sec (debug, 20% of isolated min observed
                 // 22.6M)
static constexpr double MIN_BOTH_CHANGED_CALLS_PER_SECOND
  = 2'500'000.0; // 2.5M calls/sec (debug, 20% of isolated min observed
                 // 12.5M)
#endif

namespace {

// A minimal, standalone fixture for GOSoundEnclosureShelfMapper - see
// GOTestSoundEnclosureShelfMapper.cpp's MapperFixture for the fuller
// justification of driving a real GOEnclosure with no ODF/.cmb I/O.
struct MapperFixture {
  GOConfig config;
  GOOrganModel organModel;
  GOEnclosure enclosure;
  GOSoundShelfFilterProcessor processor;
  GOSoundEnclosureShelfMapper mapper;

  MapperFixture()
    : config("GOTestPerfSoundEnclosureShelfMapper", ""),
      organModel(config),
      enclosure(organModel),
      mapper(enclosure, processor) {
    processor.EnsureSetup(MAX_OUTPUT_CHANNELS, N_FRAMES, SAMPLE_RATE);
  }

  /** Sets the enclosure to the baseline position/config and calls
   * EnsureParametersUpToDate() once, so the timed loop can start from a
   * known, already-synced state. */
  void SyncToBaseline() {
    enclosure.SetEnclosureValue(BASELINE_MIDI);
    enclosure.SetLowShelfFrequency(BASELINE_LOW_FREQUENCY);
    enclosure.SetLowShelfAttenuationDb(BASELINE_LOW_ATTENUATION_DB);
    enclosure.SetHighShelfFrequency(BASELINE_HIGH_FREQUENCY);
    enclosure.SetHighShelfAttenuationDb(BASELINE_HIGH_ATTENUATION_DB);
    mapper.EnsureParametersUpToDate();
  }
};

} // namespace

void GOTestPerfSoundEnclosureShelfMapper::TestPerfNothingChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();

  const auto start = std::chrono::high_resolution_clock::now();

  for (long iterI = 0; iterI < N_ITERATIONS; iterI++)
    fixture.mapper.EnsureParametersUpToDate();

  const auto end = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = end - start;
  const double callsPerSecond = N_ITERATIONS / elapsed.count();
  const bool isPassed = callsPerSecond >= MIN_NOTHING_CHANGED_CALLS_PER_SECOND;
  const double ratio = callsPerSecond / MIN_NOTHING_CHANGED_CALLS_PER_SECOND;

  std::cout << std::format(
    "  [{}] NothingChanged: {:.1f} calls/sec (threshold: {:.1f}, ratio: "
    "{:5.2f}x)\n",
    isPassed ? "PASS" : "FAIL",
    callsPerSecond,
    MIN_NOTHING_CHANGED_CALLS_PER_SECOND,
    ratio);
  if (!isPassed)
    m_failedTests.push_back(std::format(
      "NothingChanged: {:.1f} calls/sec < {:.1f}",
      callsPerSecond,
      MIN_NOTHING_CHANGED_CALLS_PER_SECOND));
}

void GOTestPerfSoundEnclosureShelfMapper::TestPerfOnlyHighShelfChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();

  const auto start = std::chrono::high_resolution_clock::now();

  for (long iterI = 0; iterI < N_ITERATIONS; iterI++) {
    fixture.enclosure.SetHighShelfFrequency(
      (iterI % 2 == 0) ? TOGGLED_HIGH_FREQUENCY : BASELINE_HIGH_FREQUENCY);
    fixture.mapper.EnsureParametersUpToDate();
  }

  const auto end = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = end - start;
  const double callsPerSecond = N_ITERATIONS / elapsed.count();
  const bool isPassed
    = callsPerSecond >= MIN_ONLY_HIGH_CHANGED_CALLS_PER_SECOND;
  const double ratio = callsPerSecond / MIN_ONLY_HIGH_CHANGED_CALLS_PER_SECOND;

  std::cout << std::format(
    "  [{}] OnlyHighShelfChanged: {:.1f} calls/sec (threshold: {:.1f}, "
    "ratio: {:5.2f}x)\n",
    isPassed ? "PASS" : "FAIL",
    callsPerSecond,
    MIN_ONLY_HIGH_CHANGED_CALLS_PER_SECOND,
    ratio);
  if (!isPassed)
    m_failedTests.push_back(std::format(
      "OnlyHighShelfChanged: {:.1f} calls/sec < {:.1f}",
      callsPerSecond,
      MIN_ONLY_HIGH_CHANGED_CALLS_PER_SECOND));
}

void GOTestPerfSoundEnclosureShelfMapper::TestPerfHighAndLowShelfChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();

  const auto start = std::chrono::high_resolution_clock::now();

  for (long iterI = 0; iterI < N_ITERATIONS; iterI++) {
    const bool isToggled = iterI % 2 == 0;

    fixture.enclosure.SetLowShelfFrequency(
      isToggled ? TOGGLED_LOW_FREQUENCY : BASELINE_LOW_FREQUENCY);
    fixture.enclosure.SetHighShelfFrequency(
      isToggled ? TOGGLED_HIGH_FREQUENCY : BASELINE_HIGH_FREQUENCY);
    fixture.mapper.EnsureParametersUpToDate();
  }

  const auto end = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = end - start;
  const double callsPerSecond = N_ITERATIONS / elapsed.count();
  const bool isPassed = callsPerSecond >= MIN_BOTH_CHANGED_CALLS_PER_SECOND;
  const double ratio = callsPerSecond / MIN_BOTH_CHANGED_CALLS_PER_SECOND;

  std::cout << std::format(
    "  [{}] HighAndLowShelfChanged: {:.1f} calls/sec (threshold: {:.1f}, "
    "ratio: {:5.2f}x)\n",
    isPassed ? "PASS" : "FAIL",
    callsPerSecond,
    MIN_BOTH_CHANGED_CALLS_PER_SECOND,
    ratio);
  if (!isPassed)
    m_failedTests.push_back(std::format(
      "HighAndLowShelfChanged: {:.1f} calls/sec < {:.1f}",
      callsPerSecond,
      MIN_BOTH_CHANGED_CALLS_PER_SECOND));
}

bool GOTestPerfSoundEnclosureShelfMapper::setUp() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for "
               "GOSoundEnclosureShelfMapper ==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << N_ITERATIONS << " iterations\n";
  return true;
}

bool GOTestPerfSoundEnclosureShelfMapper::tearDown() {
  std::cout << "\n========== Performance Tests Completed ==========\n";
  return true;
}

void GOTestPerfSoundEnclosureShelfMapper::run() {
  GO_RUN_TEST(TestPerfNothingChanged())
  GO_RUN_TEST(TestPerfOnlyHighShelfChanged())
  GO_RUN_TEST(TestPerfHighAndLowShelfChanged())
  // It must stay here and not in tearDown(), because an exception thrown from
  // tearDown() is not counted as a test failure. Does nothing if no test was
  // run
  if (!m_failedTests.empty()) {
    std::string errorMsg
      = std::format("{} performance test(s) failed:\n", m_failedTests.size());

    for (const auto &failedTest : m_failedTests)
      errorMsg += "  - " + failedTest + "\n";
    GOAssert(false, errorMsg);
  }
}
