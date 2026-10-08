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

#include "GOInt.h"

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
  // Debug, worst of all local and CI runs 264.6/283.7/289.8/291.1, minus 10%.
  // size=32 is dominated by fixed per-call overhead (like
  // GOTestPerfSoundShelfFilterProcessor's own bypass-path baseline) and is
  // unusually sensitive to other tests/processes contending for the
  // machine when the full suite runs: observed as low as 221.2/247.6
  // Mitems/sec under full-suite contention in two separate runs, so it
  // keeps its own wider margin instead of the shared -10%.
  {32, 200},
  {128, 250},
  {512, 260},
  {2048, 260},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_MONO_FLOAT_POLYPHASE[]
  = {
#ifdef NDEBUG
    // Release, worst of all local and CI runs 279.8/312.4/375.3/524.1, minus
    // 10%
    {32, 250},
    {128, 280},
    {512, 330},
    {2048, 470},
#else
    // Debug, worst of all local and CI runs 75.2/76.1/76.7/76.7, minus 10%
    {32, 67},
    {128, 68},
    {512, 69},
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
  // Debug, worst of all local and CI runs 252.2/259.4/259.9/260.6, minus 10%
  {32, 220},
  {128, 230},
  {512, 230},
  {2048, 230},
#endif
};

// Stereo24 (nOutChannels=2) reports Mitems/sec - 2x the frame throughput -
// see run()'s RunAndEvaluateTest() calls, isItemsPerSecond=true.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_STEREO24_POLYPHASE[] = {
#ifdef NDEBUG
  // Release, worst of all local and CI runs 132.3/132.3/131.7/132.7, minus 10%
  {32, 110},
  {128, 110},
  {512, 110},
  {2048, 110},
#else
  // Debug, worst of all local and CI runs 71.3/71.7/72.0/72.0, minus 10%
  {32, 64},
  {128, 64},
  {512, 64},
  {2048, 64},
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

static void fill_with_ramp(std::vector<float> &data) {
  for (unsigned i = 0, n = data.size(); i < n; i++)
    data[i] = (float)i;
}

static void fill_with_ramp(std::vector<GOInt24> &data) {
  for (unsigned i = 0, n = data.size(); i < n; i++)
    data[i] = (int)(i % 0x7fffff);
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

    fill_with_ramp(src);

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

    fill_with_ramp(src);

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

    fill_with_ramp(src);

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

    fill_with_ramp(src);

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

void GOTestPerfSoundResample::run() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundResample "
               "==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << GetNumIterations()
            << " iterations per buffer size\n";

  TestPerfResampleBlockMonoFloatLinear();
  TestPerfResampleBlockMonoFloatPolyphase();
  TestPerfResampleBlockStereo24Linear();
  TestPerfResampleBlockStereo24Polyphase();

  std::cout << "\n========== Performance Tests Completed ==========\n";

  ReportFailedTests();
}
