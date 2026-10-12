/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundVibratoProcessor.h"

#include <iostream>
#include <vector>

#include "sound/GOSoundDefs.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/effects/GOSoundVibratoPitchRateCurve.h"
#include "sound/effects/GOSoundVibratoProcessor.h"
#include "sound/processing/GOSoundProcessor.h"
#include "sound/processing/GOSoundProcessorState.h"

#include "../GOSoundTestSignals.h"
#include "GOTestScope.h"

const std::string GOTestPerfSoundVibratoProcessor::TEST_NAME
  = "GOTestPerfSoundVibratoProcessor";

static constexpr unsigned TEST_SAMPLE_RATE = 96000;

/* Reports Mitems/sec (MAX_OUTPUT_CHANNELS items per frame), not
 * Mframes/sec, since every call processes MAX_OUTPUT_CHANNELS channels
 * together - see run()'s RunAndEvaluateTest() calls, isItemsPerSecond=true.
 * Calibration rule throughout this file: worst observed run, minus 10%,
 * floored to 2 significant digits - from three local runs each (Release
 * and Debug), at TEST_SAMPLE_RATE, lowered where CI runs were slower. */
static constexpr GOTestPerfSoundBufferBaseline BASELINE_LINEAR[] = {
#ifdef NDEBUG
  // Release, worst of 3 runs 409.0/560.9/587.0/507.6 Mitems/sec, minus 10%
  {32, 360},
  {128, 500},
  {512, 520},
  {2048, 450},
#else
  // Debug, worst of local and CI runs 78.2/90.3/89.8/90.1 Mitems/sec, minus
  // 10%
  {32, 70},
  {128, 81},
  {512, 80},
  {2048, 81},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_POLYPHASE[] = {
#ifdef NDEBUG
  // Release, worst of local and CI runs 351.3/430.0/450.5/437.2 Mitems/sec,
  // minus 10%
  {32, 310},
  {128, 380},
  {512, 400},
  {2048, 390},
#else
  // Debug, worst of local and CI runs 35.0/39.8/41.0/41.0 Mitems/sec, minus
  // 10%
  {32, 31},
  {128, 35},
  {512, 36},
  {2048, 36},
#endif
};

/* GetResamplerPositionIncrementsForUse(): Mitems/sec like above, so the figures
 * are comparable with Process(): one call serves a period of all
 * MAX_OUTPUT_CHANNELS channels, i.e. MAX_OUTPUT_CHANNELS items per frame (the
 * call itself is the same for any number of channels). The same calibration
 * rule as above. */
static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_INCREMENTS_NO_VIOLATION[]
  = {
/* The call is O(1) here (a few ns), so the figures are noisy and the margin
 * is wider than the usual 10% */
#ifdef NDEBUG
    // Release, worst of local and CI runs 11448/47452/189940/734676 Mitems/sec
    {32, 10000},
    {128, 42000},
    {512, 170000},
    {2048, 660000},
#else
    // Debug, worst of local and CI runs 8602/29391/116640/425698 Mitems/sec
    {32, 6000},
    {128, 26000},
    {512, 100000},
    {2048, 380000},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_INCREMENTS_WITH_RESCALE[]
  = {
#ifdef NDEBUG
    // Release, worst of local and CI runs 1050.0/1837.1/1687.0/1068.5
    // Mitems/sec, minus 10%, floored to 2 significant figures (sizes 128 and
    // 512 rebaselined 2026-10-31: the run on an Intel Xeon Platinum 8573C was
    // 8..20% below all the earlier ones)
    {32, 940},
    {128, 1600},
    {512, 1500},
    {2048, 960},
#else
    // Debug, worst of local and CI runs 613.8/820.6/840.4/543.8 Mitems/sec,
    // minus 10% (sizes 32 and 128 lowered after local runs 533.2 and 618.4,
    // size 512 after CI runs 722.7..725.4)
    {32, 470},
    {128, 550},
    {512, 650},
    {2048, 480},
#endif
};

template <unsigned N>
void GOTestPerfSoundVibratoProcessor::RunActiveBaseline(
  const std::string &functionName,
  const GOTestPerfSoundBufferBaseline (&baselines)[N],
  GOSoundResample::InterpolationType interpolationType) {
  GOSoundResample resample;

  for (const GOTestPerfSoundBufferBaseline &baseline : baselines) {
    GOSoundVibratoProcessor processor(resample, interpolationType);

    // The crossfades are disabled: the steady ACTIVE phase is measured
    processor.SetActivationCrossfadeMs(0);
    processor.SetDeactivationCrossfadeMs(0);
    processor.EnsureSetup(
      MAX_OUTPUT_CHANNELS, baseline.m_BufferSize, TEST_SAMPLE_RATE);

    GOSoundProcessor &untypedProcessor = processor;
    std::unique_ptr<GOSoundProcessorState> oState
      = untypedProcessor.CreateState();
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buffer, MAX_OUTPUT_CHANNELS, baseline.m_BufferSize)
    const std::vector<float> deviations(baseline.m_BufferSize, 0.0f);
    GOSoundVibratoPitchRateCurve curve;

    curve.Build(
      deviations.data(), baseline.m_BufferSize, baseline.m_BufferSize);
    // A zero-deviation curve is constant, so one chunk serves every period
    processor.SetPitchRateChunk(curve.GetChunkDescription(
      GOSoundVibratoPitchRateCurve::LoopPosition(baseline.m_BufferSize)));
    GOSoundTestSignals::fillWithRamp(buffer);

    RunAndEvaluateTest(
      functionName,
      baseline,
      [&untypedProcessor, &oState, &buffer]() {
        untypedProcessor.Process(*oState, buffer);
      },
      MAX_OUTPUT_CHANNELS,
      true);
  }
}

void GOTestPerfSoundVibratoProcessor::TestPerfProcessActiveLinear() {
  std::cout << "\nPerformance test: Process (active, Linear)\n";

  RunActiveBaseline(
    "ProcessActiveLinear",
    BASELINE_LINEAR,
    GOSoundResample::GO_LINEAR_INTERPOLATION);
}

void GOTestPerfSoundVibratoProcessor::TestPerfProcessActivePolyphase() {
  std::cout << "\nPerformance test: Process (active, Polyphase)\n";

  RunActiveBaseline(
    "ProcessActivePolyphase",
    BASELINE_POLYPHASE,
    GOSoundResample::GO_POLYPHASE_INTERPOLATION);
}

template <unsigned N>
void GOTestPerfSoundVibratoProcessor::RunIncrementsBaseline(
  const std::string &functionName,
  const GOTestPerfSoundBufferBaseline (&baselines)[N],
  unsigned rateMultiplier,
  unsigned nExtraFramesToRead) {
  GOSoundResample resample;

  for (const GOTestPerfSoundBufferBaseline &baseline : baselines) {
    const unsigned nFrames = baseline.m_BufferSize;
    GOSoundVibratoProcessor processor(resample);

    processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE);

    /* ring - L == nMaxLagFrames + nFrames == availRead + availWrite */
    const unsigned nReadAndWriteFrames = processor.m_NDelayBufferRingFrames
      - GOSoundVibratoProcessorState::N_LOOKAHEAD_FRAMES;
    const unsigned availRead = nFrames + nExtraFramesToRead;
    const unsigned availWrite = nReadAndWriteFrames - availRead;
    const unsigned fraction = 1234;
    const std::vector<uint16_t> rates(
      nFrames, rateMultiplier * GOSoundResample::UPSAMPLE_FACTOR);
    std::vector<uint16_t> tmpIncrements(nFrames);
    unsigned long sink = 0;

    processor.SetPitchRateChunk(
      {rates.data(),
       nFrames,
       rateMultiplier * GOSoundResample::UPSAMPLE_FACTOR * nFrames});

    RunAndEvaluateTest(
      functionName,
      baseline,
      [&]() {
        // The sum keeps the optimiser from dropping the call
        sink += *processor.GetResamplerPositionIncrementsForUse(
          fraction, availRead, availWrite, tmpIncrements.data());
      },
      MAX_OUTPUT_CHANNELS,
      true);
    if (sink == 1)
      std::cout << "";
  }
}

void GOTestPerfSoundVibratoProcessor::TestPerfIncrementsNoViolation() {
  std::cout << "\nPerformance test: GetResamplerPositionIncrementsForUse "
               "(no violation)\n";
  // The chunk advances nFrames frames, the window is up to nFrames + 4000
  RunIncrementsBaseline(
    "IncrementsNoViolation", BASELINE_INCREMENTS_NO_VIOLATION, 1, 4000);
}

void GOTestPerfSoundVibratoProcessor::TestPerfIncrementsWithRescale() {
  std::cout << "\nPerformance test: GetResamplerPositionIncrementsForUse "
               "(with rescale)\n";
  // The chunk advances 2 * nFrames frames, the window is only nFrames + 1
  RunIncrementsBaseline(
    "IncrementsWithRescale", BASELINE_INCREMENTS_WITH_RESCALE, 2, 1);
}

bool GOTestPerfSoundVibratoProcessor::setUp() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundVibratoProcessor "
               "==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << GetNFrames() << " frames per measurement\n";
  return true;
}

void GOTestPerfSoundVibratoProcessor::run() {
  GO_RUN_TEST(TestPerfProcessActiveLinear())
  GO_RUN_TEST(TestPerfProcessActivePolyphase())
  GO_RUN_TEST(TestPerfIncrementsNoViolation())
  GO_RUN_TEST(TestPerfIncrementsWithRescale())
  // It must stay here and not in tearDown(), because an exception thrown from
  // tearDown() is not counted as a test failure. Does nothing if no test was
  // run
  ReportFailedTests();
}

bool GOTestPerfSoundVibratoProcessor::tearDown() {
  std::cout << "\n========== Performance Tests Completed ==========\n";
  return true;
}
