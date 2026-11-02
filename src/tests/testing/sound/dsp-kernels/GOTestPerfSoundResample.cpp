/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundResample.h"

#include <cmath>
#include <iostream>
#include <vector>

#include "sound/dsp-kernels/GOSoundResample.h"

#include "../GOSoundTestSignals.h"
#include "GOInt.h"
#include "GOTestScope.h"

const std::string GOTestPerfSoundResample::TEST_NAME
  = "GOTestPerfSoundResample";

// Baseline values are the worst of all local isolated runs and GitHub
// Actions CI runs observed so far, minus a 10% margin and rounded down to two
// significant digits (the per-table comments list the raw worst values, before
// the margin), calibrated separately
// for Release/Debug since Debug's unoptimized code runs at roughly a third of
// Release's throughput on a developer machine -
// a single shared table would either be too loose in Release or
// spuriously fail in Debug. The smallest (size=32) Debug case in any of the
// tables below is occasionally seen to dip further when the full test suite
// runs (other tests contending for the machine); it is dominated by fixed
// per-call overhead, the same effect GOTestPerfSoundShelfFilterProcessor's
// baseline comment documents for its own bypass-path case - a rerun with
// `--perf-only` isolates this test and does not show the dip.
// This file reports Mitems/sec throughout (see each run()/Test*() call's
// RunAndEvaluateTest(..., nOutChannels, /* isItemsPerSecond= */ true)) -
// for the mono cases (nOutChannels=1) that numerically equals Mframes/sec,
// so their tables are unaffected in value; the Stereo24 tables' values are
// 2x their own frame throughput, noted individually below.
// A further recalibration pass (2026-10-31, against the CI runs of both
// GrandOrgue/grandorgue and oleg68/GrandOrgue-official made 2026-10-17..31,
// after commit 341191074 changed the iteration count) re-derived every table
// from the worst throughput observed in that window with the same -10% margin.
// Format: {buffer_size, min_MItems_per_second} - see
// GOTestPerfSoundBufferBase::RunAndEvaluateTest().
static constexpr GOTestPerfSoundBufferBaseline BASELINE_MONO_FLOAT_LINEAR[] = {
#ifdef NDEBUG
  // Release, worst of all local and CI runs 702.9/744.8/656.9/676.8, minus 10%
  {32, 630},
  {128, 670},
  {512, 590},
  {2048, 600},
#else
  // Debug, worst of all local and CI runs 211.4/217.3/219.1/228.5, minus 10%.
  // size=32 is dominated by fixed per-call overhead (like
  // GOTestPerfSoundShelfFilterProcessor's own bypass-path baseline) and is
  // unusually sensitive to other tests/processes contending for the
  // machine when the full suite runs: observed as low as 221.2/247.6
  // Mitems/sec under full-suite contention in two separate runs, so it
  // keeps its own wider margin instead of the shared -10%.
  {32, 190},
  {128, 160}, // a CI run 185.8 Mitems/sec, minus 10%
  {512, 190},
  {2048, 200},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_MONO_FLOAT_POLYPHASE[]
  = {
#ifdef NDEBUG
    // Release, worst of all local and CI runs 279.8/312.4/375.3/507.5, minus
    // 10%, floored to 2 significant figures (rebaselined 2026-10-31 against
    // the 2026-10-17..31 CI runs of both repos)
    {32, 250},
    {128, 280},
    {512, 330},
    {2048, 450},
#else
    // Debug, worst of all local and CI runs 72.4/74.1/75.2/76.7, minus 10%,
    // floored to 2 significant figures (rebaselined 2026-10-31 against the
    // 2026-10-17..31 CI runs of both repos)
    {32, 65},
    {128, 66},
    {512, 67},
    {2048, 69},
#endif
};

// Stereo24 (nOutChannels=2) reports Mitems/sec - 2x the frame throughput -
// see run()'s RunAndEvaluateTest() calls, isItemsPerSecond=true.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_STEREO24_LINEAR[] = {
#ifdef NDEBUG
  // Release, worst of all local and CI runs 540.4/589.5/561.2/602.3, minus 10%
  {32, 480},
  {128, 530},
  {512, 500},
  {2048, 540},
#else
  // Debug, worst of all local and CI runs 221.2/228.7/228.8/231.5, minus 10%
  {32, 190},
  {128, 200},
  {512, 200},
  {2048, 200},
#endif
};

// Stereo24 (nOutChannels=2) reports Mitems/sec - 2x the frame throughput -
// see run()'s RunAndEvaluateTest() calls, isItemsPerSecond=true.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_STEREO24_POLYPHASE[] = {
#ifdef NDEBUG
  // Release, worst of all local and CI runs 117.4/114.8/114.0/114.6, minus 10%
  {32, 100},
  {128, 100},
  {512, 100},
  {2048, 100},
#else
  // Debug, worst of all local and CI runs 47.2/47.8/47.3/47.5, minus 10%,
  // floored to 2 significant figures (rebaselined 2026-10-31 against the
  // 2026-10-17..31 CI runs of both repos)
  {32, 42},
  {128, 43},
  {512, 42},
  {2048, 42},
#endif
};

// A non-integral resampling factor (one equal-tempered semitone) so that the
// fractional part of the resampling position changes on every output sample,
// as it does in production, where the factor is
// GetRandomFactor() * tuning / outputSampleRate. With an integral factor the
// fraction would stay zero and only r_coefs[0] would ever be read.
static constexpr float RESAMPLING_FACTOR = 1.0594631f;

/**
 * Calculates a source length sufficient for resampling nOutFrames output
 * frames with RESAMPLING_FACTOR. The +1 covers the fractional part carried
 * over between iterations.
 */
static unsigned compute_src_frames(unsigned nOutFrames, unsigned vectorLength) {
  return (unsigned)std::ceil(nOutFrames * RESAMPLING_FACTOR) + vectorLength + 1;
}

void GOTestPerfSoundResample::TestPerfResampleBlockMonoFloatLinear() {
  std::cout
    << "\nPerformance test: LinearResampler::ResampleBlock (mono float)\n";

  GOSoundResample resampler;
  GOSoundResample::LinearResampler linearResampler(resampler);

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_MONO_FLOAT_LINEAR) {
    const unsigned nOutFrames = baseline.m_BufferSize;
    const unsigned nSrcFrames = compute_src_frames(
      nOutFrames, GOSoundResample::LinearResampler::VECTOR_LENGTH);
    std::vector<float> src(nSrcFrames);
    std::vector<float> out(nOutFrames);

    GOSoundTestSignals::fillWithRamp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(RESAMPLING_FACTOR);

    GOSoundResample::PtrFrameVector<float, float, 1> fV(src.data());

    RunAndEvaluateTest(
      "ResampleBlockMonoFloatLinear",
      baseline,
      [&resamplingPos, &linearResampler, &fV, &out, nOutFrames]() {
        resamplingPos.SetIndex(0);
        linearResampler
          .ResampleBlock<GOSoundResample::PtrFrameVector<float, float, 1>, 1>(
            resamplingPos, fV, out.data(), nOutFrames);
      },
      1,
      true);
  }
}

void GOTestPerfSoundResample::TestPerfResampleBlockMonoFloatPolyphase() {
  std::cout
    << "\nPerformance test: PolyphaseResampler::ResampleBlock (mono float)\n";

  GOSoundResample resampler;
  GOSoundResample::PolyphaseResampler polyphaseResampler(resampler);

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_MONO_FLOAT_POLYPHASE) {
    const unsigned nOutFrames = baseline.m_BufferSize;
    const unsigned nSrcFrames = compute_src_frames(
      nOutFrames, GOSoundResample::PolyphaseResampler::VECTOR_LENGTH);
    std::vector<float> src(nSrcFrames);
    std::vector<float> out(nOutFrames);

    GOSoundTestSignals::fillWithRamp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(RESAMPLING_FACTOR);

    GOSoundResample::PtrFrameVector<float, float, 1> fV(src.data());

    RunAndEvaluateTest(
      "ResampleBlockMonoFloatPolyphase",
      baseline,
      [&resamplingPos, &polyphaseResampler, &fV, &out, nOutFrames]() {
        resamplingPos.SetIndex(0);
        polyphaseResampler
          .ResampleBlock<GOSoundResample::PtrFrameVector<float, float, 1>, 1>(
            resamplingPos, fV, out.data(), nOutFrames);
      },
      1,
      true);
  }
}

void GOTestPerfSoundResample::TestPerfResampleBlockStereo24Linear() {
  std::cout
    << "\nPerformance test: LinearResampler::ResampleBlock (stereo GOInt24)\n";

  GOSoundResample resampler;
  GOSoundResample::LinearResampler linearResampler(resampler);

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_STEREO24_LINEAR) {
    const unsigned nOutFrames = baseline.m_BufferSize;
    const unsigned nSrcFrames = compute_src_frames(
      nOutFrames, GOSoundResample::LinearResampler::VECTOR_LENGTH);
    std::vector<GOInt24> src(nSrcFrames * 2);
    std::vector<float> out(nOutFrames * 2);

    GOSoundTestSignals::fillWithRamp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(RESAMPLING_FACTOR);

    GOSoundResample::PtrFrameVector<GOInt24, int, 2> fV(src.data());

    RunAndEvaluateTest(
      "ResampleBlockStereo24Linear",
      baseline,
      [&resamplingPos, &linearResampler, &fV, &out, nOutFrames]() {
        resamplingPos.SetIndex(0);
        linearResampler
          .ResampleBlock<GOSoundResample::PtrFrameVector<GOInt24, int, 2>, 2>(
            resamplingPos, fV, out.data(), nOutFrames);
      },
      2,
      true);
  }
}

void GOTestPerfSoundResample::TestPerfResampleBlockStereo24Polyphase() {
  std::cout << "\nPerformance test: PolyphaseResampler::ResampleBlock "
               "(stereo GOInt24)\n";

  GOSoundResample resampler;
  GOSoundResample::PolyphaseResampler polyphaseResampler(resampler);

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_STEREO24_POLYPHASE) {
    const unsigned nOutFrames = baseline.m_BufferSize;
    const unsigned nSrcFrames = compute_src_frames(
      nOutFrames, GOSoundResample::PolyphaseResampler::VECTOR_LENGTH);
    std::vector<GOInt24> src(nSrcFrames * 2);
    std::vector<float> out(nOutFrames * 2);

    GOSoundTestSignals::fillWithRamp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(RESAMPLING_FACTOR);

    GOSoundResample::PtrFrameVector<GOInt24, int, 2> fV(src.data());

    RunAndEvaluateTest(
      "ResampleBlockStereo24Polyphase",
      baseline,
      [&resamplingPos, &polyphaseResampler, &fV, &out, nOutFrames]() {
        resamplingPos.SetIndex(0);
        polyphaseResampler
          .ResampleBlock<GOSoundResample::PtrFrameVector<GOInt24, int, 2>, 2>(
            resamplingPos, fV, out.data(), nOutFrames);
      },
      2,
      true);
  }
}

// ResampleBlockVariableRatePlanar(): stereo only (reports Mitems/sec: 2x the
// frame throughput) - mono would run the channel-outer/frame-inner loop for a
// single channel, identical total work to a constant-rate interleaved call, so
// it would not exercise the one thing this path does differently: replaying the
// position trajectory once per channel (see ResampleBlockVariableRatePlanar()'s
// doc comment).
static constexpr unsigned N_PLANAR_PERF_CHANNELS = 2;

// Calibration rule throughout this file: worst observed run, minus 10%,
// floored to 2 significant digits.
static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_VARIABLE_RATE_PLANAR_LINEAR[]
  = {
#ifdef NDEBUG
    // Release, worst of local runs and GrandOrgue/grandorgue CI runs
    // 728.4/770.7/785.1/788.5, minus 10%, floored to 2 significant figures
    // (rebaselined 2026-10-31 against the 2026-10-17..31 CI runs of both
    // repos).
    {32, 650},
    {128, 690},
    {512, 700},
    {2048, 700},
#else
    // Debug, single local run 217.4/233.3/236.2/235.2.
    {32, 190},
    {128, 200},
    {512, 210},
    {2048, 210},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_VARIABLE_RATE_PLANAR_POLYPHASE[]
  = {
#ifdef NDEBUG
    // Release, worst of local runs and of the CI runs on both repos
    // 296.6/297.1/296.4/296.4, minus 10%, floored to 2 significant figures (the
    // 2026-10-31 run on an Intel Xeon Platinum 8573C was 15..16% below all the
    // earlier runs).
    {32, 260},
    {128, 260},
    {512, 260},
    {2048, 260},
#else
    // Debug, worst of local runs and GrandOrgue/grandorgue CI runs
    // 50.8/52.5/51.0/52.0, minus 10%, floored to 2 significant figures
    // (rebaselined 2026-10-31 against the 2026-10-17..31 CI runs of both
    // repos).
    {32, 45},
    {128, 47},
    {512, 45},
    {2048, 46},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_CONSTANT_RATE_PLANAR_LINEAR[]
  = {
#ifdef NDEBUG
    // Release, worst of local and CI runs 661.5/685.1/684.7/690.8, minus 10%,
    // floored to 2 significant figures (rebaselined 2026-10-31 against the
    // 2026-10-17..31 CI runs of both repos).
    {32, 590},
    {128, 610},
    {512, 610},
    {2048, 620},
#else
    // Debug, worst of local and CI runs 87.3/90.3/91.1/91.4, minus 10%.
    {32, 78},
    {128, 81},
    {512, 81},
    {2048, 82},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_CONSTANT_RATE_PLANAR_POLYPHASE[]
  = {
#ifdef NDEBUG
    // Release, worst of local and CI runs 478.9/503.7/505.5/510.7, minus 10%,
    // floored to 2 significant figures (rebaselined 2026-10-31 against the
    // 2026-10-17..31 CI runs of both repos).
    {32, 430},
    {128, 450},
    {512, 450},
    {2048, 450},
#else
    // Debug, worst of local and CI runs 40.1/40.9/41.1/41.3, minus 10%.
    {32, 36},
    {128, 36},
    {512, 36},
    {2048, 37},
#endif
};

void GOTestPerfSoundResample::TestPerfResampleBlockVariableRatePlanarLinear() {
  std::cout << "\nPerformance test: "
               "LinearResampler::ResampleBlockVariableRatePlanar (stereo)\n";

  GOSoundResample resampler;
  GOSoundResample::LinearResampler linearResampler(resampler);

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_VARIABLE_RATE_PLANAR_LINEAR) {
    const unsigned nOutFrames = baseline.m_BufferSize;
    const unsigned nSrcFrames
      = nOutFrames + GOSoundResample::LinearResampler::VECTOR_LENGTH;
    std::vector<float> src(nSrcFrames * N_PLANAR_PERF_CHANNELS);
    // Planar layout: channel c's frame f is at out[c * nOutFrames + f].
    std::vector<float> out(nOutFrames * N_PLANAR_PERF_CHANNELS);

    GOSoundTestSignals::fillWithRamp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(1.0f);

    GOSoundResample::PtrFrameVector<float, float, N_PLANAR_PERF_CHANNELS> fV(
      src.data());
    const std::vector<uint16_t> increments(
      nOutFrames, (uint16_t)resamplingPos.GetFractionIncrement());

    RunAndEvaluateTest(
      "ResampleBlockVariableRatePlanarLinear",
      baseline,
      [&resamplingPos, &linearResampler, &fV, &increments, &out, nOutFrames]() {
        resamplingPos.SetIndex(0);
        linearResampler.ResampleBlockVariableRatePlanar<
          GOSoundResample::
            PtrFrameVector<float, float, N_PLANAR_PERF_CHANNELS>>(
          resamplingPos,
          fV,
          increments.data(),
          nOutFrames,
          N_PLANAR_PERF_CHANNELS,
          out.data(),
          nOutFrames);
      },
      N_PLANAR_PERF_CHANNELS,
      true);
  }
}

void GOTestPerfSoundResample::
  TestPerfResampleBlockVariableRatePlanarPolyphase() {
  std::cout << "\nPerformance test: "
               "PolyphaseResampler::ResampleBlockVariableRatePlanar "
               "(stereo)\n";

  GOSoundResample resampler;
  GOSoundResample::PolyphaseResampler polyphaseResampler(resampler);

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_VARIABLE_RATE_PLANAR_POLYPHASE) {
    const unsigned nOutFrames = baseline.m_BufferSize;
    const unsigned nSrcFrames
      = nOutFrames + GOSoundResample::PolyphaseResampler::VECTOR_LENGTH;
    std::vector<float> src(nSrcFrames * N_PLANAR_PERF_CHANNELS);
    std::vector<float> out(nOutFrames * N_PLANAR_PERF_CHANNELS);

    GOSoundTestSignals::fillWithRamp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(1.0f);

    GOSoundResample::PtrFrameVector<float, float, N_PLANAR_PERF_CHANNELS> fV(
      src.data());
    const std::vector<uint16_t> increments(
      nOutFrames, (uint16_t)resamplingPos.GetFractionIncrement());

    RunAndEvaluateTest(
      "ResampleBlockVariableRatePlanarPolyphase",
      baseline,
      [&resamplingPos,
       &polyphaseResampler,
       &fV,
       &increments,
       &out,
       nOutFrames]() {
        resamplingPos.SetIndex(0);
        polyphaseResampler.ResampleBlockVariableRatePlanar<
          GOSoundResample::
            PtrFrameVector<float, float, N_PLANAR_PERF_CHANNELS>>(
          resamplingPos,
          fV,
          increments.data(),
          nOutFrames,
          N_PLANAR_PERF_CHANNELS,
          out.data(),
          nOutFrames);
      },
      N_PLANAR_PERF_CHANNELS,
      true);
  }
}

/**
 * Measures ResampleBlockConstantRatePlanar() of a stereo RingPlanarFrameVector
 * at the rate 1.0 with a non-zero starting fraction (the frozen-lag case of
 * GOSoundVibratoProcessor).
 */
template <class ResamplerT, size_t nBaselines>
void GOTestPerfSoundResample::run_constant_rate_planar(
  const char *name,
  const ResamplerT &resampler,
  const GOTestPerfSoundBufferBaseline (&baselines)[nBaselines]) {
  using Ring = GOSoundResample::RingPlanarFrameVector<float, float>;

  for (const GOTestPerfSoundBufferBaseline &baseline : baselines) {
    const unsigned nOutFrames = baseline.m_BufferSize;
    const unsigned nTailFrames = ResamplerT::VECTOR_LENGTH;
    const unsigned nRingFrames = nOutFrames + nTailFrames;
    const unsigned nPhysicalFrames = nRingFrames + nTailFrames;
    std::vector<float> src(nPhysicalFrames * N_PLANAR_PERF_CHANNELS);
    std::vector<float> out(nOutFrames * N_PLANAR_PERF_CHANNELS);

    GOSoundTestSignals::fillWithRamp(src);

    Ring ring(src.data(), N_PLANAR_PERF_CHANNELS, nRingFrames, nTailFrames);
    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(1.0f);
    resamplingPos.Inc(1000); // a non-zero fraction

    // Every iteration starts from the same position
    const GOSoundResample::ResamplingPosition startPos = resamplingPos;

    RunAndEvaluateTest(
      name,
      baseline,
      [&resamplingPos, &startPos, &resampler, &ring, &out, nOutFrames]() {
        resamplingPos = startPos;
        resampler.template ResampleBlockConstantRatePlanar<Ring>(
          resamplingPos,
          ring,
          nOutFrames,
          N_PLANAR_PERF_CHANNELS,
          out.data(),
          nOutFrames);
      },
      N_PLANAR_PERF_CHANNELS,
      true);
  }
}

void GOTestPerfSoundResample::TestPerfResampleBlockConstantRatePlanarLinear() {
  std::cout << "\nPerformance test: "
               "LinearResampler::ResampleBlockConstantRatePlanar (stereo)\n";

  GOSoundResample resample;

  run_constant_rate_planar(
    "ResampleBlockConstantRatePlanarLinear",
    GOSoundResample::LinearResampler(resample),
    BASELINE_CONSTANT_RATE_PLANAR_LINEAR);
}

void GOTestPerfSoundResample::
  TestPerfResampleBlockConstantRatePlanarPolyphase() {
  std::cout << "\nPerformance test: "
               "PolyphaseResampler::ResampleBlockConstantRatePlanar "
               "(stereo)\n";

  GOSoundResample resample;

  run_constant_rate_planar(
    "ResampleBlockConstantRatePlanarPolyphase",
    GOSoundResample::PolyphaseResampler(resample),
    BASELINE_CONSTANT_RATE_PLANAR_POLYPHASE);
}

bool GOTestPerfSoundResample::setUp() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundResample "
               "==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << GetNFrames() << " frames per measurement\n";
  return true;
}

void GOTestPerfSoundResample::run() {
  GO_RUN_TEST(TestPerfResampleBlockMonoFloatLinear())
  GO_RUN_TEST(TestPerfResampleBlockMonoFloatPolyphase())
  GO_RUN_TEST(TestPerfResampleBlockStereo24Linear())
  GO_RUN_TEST(TestPerfResampleBlockStereo24Polyphase())
  GO_RUN_TEST(TestPerfResampleBlockVariableRatePlanarLinear())
  GO_RUN_TEST(TestPerfResampleBlockVariableRatePlanarPolyphase())
  GO_RUN_TEST(TestPerfResampleBlockConstantRatePlanarLinear())
  GO_RUN_TEST(TestPerfResampleBlockConstantRatePlanarPolyphase())
  // It must stay here and not in tearDown(), because an exception thrown from
  // tearDown() is not counted as a test failure. Does nothing if no test was
  // run
  ReportFailedTests();
}

bool GOTestPerfSoundResample::tearDown() {
  std::cout << "\n========== Performance Tests Completed ==========\n";
  return true;
}
