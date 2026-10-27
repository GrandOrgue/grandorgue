/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundShelfFilterProcessor.h"

#include <iostream>

#include "sound/GOSoundDefs.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/effects/GOSoundShelfFilterProcessor.h"
#include "sound/processing/GOSoundProcessor.h"
#include "sound/processing/GOSoundProcessorState.h"

#include "GOTestScope.h"

const std::string GOTestPerfSoundShelfFilterProcessor::TEST_NAME
  = "GOTestPerfSoundShelfFilterProcessor";

static constexpr unsigned TEST_SAMPLE_RATE = 96000;

// Baseline values are set at roughly 20% of the minimum observed on a local
// Debug run (2026-09-01: 2937/12179/48719/188967 Mframes/sec in isolation),
// a wider margin than the buffer perf tests' usual -10% because this
// bypass-path case is dominated by fixed per-call overhead rather than real
// per-frame work, so it is unusually sensitive to other tests/processes
// contending for the machine when the full suite runs (observed ~0.45-0.5x
// of the isolated numbers under that contention). No separate Release
// numbers are available yet, so the Release thresholds below reuse the same
// Debug-derived floor rather than guessing a higher one.
// TODO: recalibrate once this test has run on CI, separately for each build
// type.
// Format: {buffer_size, min_MFrames_per_second} - see
// GOTestPerfSoundBufferBase::RunAndEvaluateTest(), which reports and
// compares against this table in Mframes/sec.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_BOTH_NOOP[] = {
#ifdef NDEBUG
  {32, 580},    // 580 Mframes/sec (20% of isolated min observed 2937.3)
  {128, 2400},  // 2400 Mframes/sec (20% of isolated min observed 12179.6)
  {512, 9700},  // 9700 Mframes/sec (20% of isolated min observed 48719.2)
  {2048, 37700} // 37700 Mframes/sec (20% of isolated min observed 188966.7)
#else
  {32, 580},    // 580 Mframes/sec (debug, 20% of isolated min observed 2937.3)
  {128, 2400},  // 2400 Mframes/sec (debug, 20% of isolated min observed
                // 12179.6)
  {512, 9700},  // 9700 Mframes/sec (debug, 20% of isolated min observed
                // 48719.2)
  {2048, 37700} // 37700 Mframes/sec (debug, 20% of isolated min observed
                // 188966.7)
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ONLY_LOW_CONFIGURED[]
  = {
#ifdef NDEBUG
    {32, 25},  // 25 Mframes/sec (~50% of isolated min observed 53.2)
    {128, 20}, // 20 Mframes/sec (~44% of isolated min observed 45.5)
    {512, 20}, // 20 Mframes/sec (~46% of isolated min observed 43.1)
    {2048, 20} // 20 Mframes/sec (~45% of isolated min observed 44.1)
#else
    {32, 25},   // 25 Mframes/sec (debug, ~50% of isolated min observed 53.2)
    {128, 20},  // 20 Mframes/sec (debug, ~44% of isolated min observed 45.5)
    {512, 20},  // 20 Mframes/sec (debug, ~46% of isolated min observed 43.1)
    {2048, 20}  // 20 Mframes/sec (debug, ~45% of isolated min observed 44.1)
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_BOTH_CONFIGURED[] = {
#ifdef NDEBUG
  {32, 24},  // 24 Mframes/sec (~49% of isolated min observed 49.3)
  {128, 24}, // 24 Mframes/sec (~49% of isolated min observed 49.2)
  {512, 20}, // 20 Mframes/sec (~48% of isolated min observed 41.7)
  {2048, 20} // 20 Mframes/sec (~44% of isolated min observed 45.3)
#else
  {32, 24},     // 24 Mframes/sec (debug, ~49% of isolated min observed 49.3)
  {128, 24},    // 24 Mframes/sec (debug, ~49% of isolated min observed 49.2)
  {512, 20},    // 20 Mframes/sec (debug, ~48% of isolated min observed 41.7)
  {2048, 20}    // 20 Mframes/sec (debug, ~44% of isolated min observed 45.3)
#endif
};

static void fill_with_alternating_signal(GOSoundBufferPlanarMutable &buffer) {
  for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
    buffer.GetData()[itemI] = (itemI % 2 == 0) ? 0.5f : -0.5f;
}

void GOTestPerfSoundShelfFilterProcessor::TestPerfProcessBothBandsNoop() {
  std::cout << "\nPerformance test: Process (both bands noop)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline : BASELINE_BOTH_NOOP) {
    GOSoundShelfFilterProcessor processor;

    processor.EnsureSetup(
      MAX_OUTPUT_CHANNELS, baseline.m_BufferSize, TEST_SAMPLE_RATE);

    GOSoundProcessor &untypedProcessor = processor;
    std::unique_ptr<GOSoundProcessorState> oState
      = untypedProcessor.CreateState();
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buffer, MAX_OUTPUT_CHANNELS, baseline.m_BufferSize)

    fill_with_alternating_signal(buffer);

    RunAndEvaluateTest(
      "ProcessBothNoop", baseline, [&untypedProcessor, &oState, &buffer]() {
        untypedProcessor.Process(*oState, buffer);
      });
  }
}

void GOTestPerfSoundShelfFilterProcessor::
  TestPerfProcessOnlyLowBandConfigured() {
  std::cout << "\nPerformance test: Process (only low band configured)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_ONLY_LOW_CONFIGURED) {
    GOSoundShelfFilterProcessor processor;

    processor.SetLowShelf(200, 6);
    processor.EnsureSetup(
      MAX_OUTPUT_CHANNELS, baseline.m_BufferSize, TEST_SAMPLE_RATE);

    GOSoundProcessor &untypedProcessor = processor;
    std::unique_ptr<GOSoundProcessorState> oState
      = untypedProcessor.CreateState();
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buffer, MAX_OUTPUT_CHANNELS, baseline.m_BufferSize)

    fill_with_alternating_signal(buffer);

    RunAndEvaluateTest(
      "ProcessOnlyLow", baseline, [&untypedProcessor, &oState, &buffer]() {
        untypedProcessor.Process(*oState, buffer);
      });
  }
}

void GOTestPerfSoundShelfFilterProcessor::TestPerfProcessBothBandsConfigured() {
  std::cout << "\nPerformance test: Process (both bands configured)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_BOTH_CONFIGURED) {
    GOSoundShelfFilterProcessor processor;

    processor.SetLowShelf(200, 6);
    processor.SetHighShelf(5000, -6);
    processor.EnsureSetup(
      MAX_OUTPUT_CHANNELS, baseline.m_BufferSize, TEST_SAMPLE_RATE);

    GOSoundProcessor &untypedProcessor = processor;
    std::unique_ptr<GOSoundProcessorState> oState
      = untypedProcessor.CreateState();
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buffer, MAX_OUTPUT_CHANNELS, baseline.m_BufferSize)

    fill_with_alternating_signal(buffer);

    RunAndEvaluateTest(
      "ProcessBothConfigured",
      baseline,
      [&untypedProcessor, &oState, &buffer]() {
        untypedProcessor.Process(*oState, buffer);
      });
  }
}

bool GOTestPerfSoundShelfFilterProcessor::setUp() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for "
               "GOSoundShelfFilterProcessor ==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << GetNFrames() << " frames per measurement\n";
  return true;
}

void GOTestPerfSoundShelfFilterProcessor::run() {
  GO_RUN_TEST(TestPerfProcessBothBandsNoop())
  GO_RUN_TEST(TestPerfProcessOnlyLowBandConfigured())
  GO_RUN_TEST(TestPerfProcessBothBandsConfigured())
  // It must stay here and not in tearDown(), because an exception thrown from
  // tearDown() is not counted as a test failure. Does nothing if no test was
  // run
  ReportFailedTests();
}

bool GOTestPerfSoundShelfFilterProcessor::tearDown() {
  std::cout << "\n========== Performance Tests Completed ==========\n";
  return true;
}
