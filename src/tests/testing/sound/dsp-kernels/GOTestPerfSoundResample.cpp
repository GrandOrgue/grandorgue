/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundResample.h"

#include <iostream>
#include <vector>

#include "sound/dsp-kernels/GOSoundResample.h"

const std::string GOTestPerfSoundResample::TEST_NAME
  = "GOTestPerfSoundResample";

// Baseline values are the worst observed run minus a 10% margin, calibrated
// separately for Release/Debug since Debug's unoptimized code runs at roughly
// a third of Release's throughput on a developer machine.
// This file reports Mitems/sec (see RunAndEvaluateTest(...,
// /* isItemsPerSecond= */ true)): 2x the frame throughput for stereo.
// Format: {buffer_size, min_MItems_per_second} - see
// GOTestPerfSoundBufferBase::RunAndEvaluateTest().

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
    // 802.3/810.0/788.9/802.0, minus 10%.
    {32, 720},
    {128, 720},
    {512, 710},
    {2048, 720},
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
    // Release, worst of local runs and GrandOrgue/grandorgue CI runs
    // 378.9/384.5/395.3/393.8, minus 10%.
    {32, 340},
    {128, 340},
    {512, 350},
    {2048, 350},
#else
    // Debug, worst of local runs and GrandOrgue/grandorgue CI runs
    // 59.7/63.1/63.7/68.9, minus 10%.
    {32, 53},
    {128, 56},
    {512, 57},
    {2048, 62},
#endif
};

static void fill_with_ramp(std::vector<float> &data) {
  for (unsigned i = 0, n = data.size(); i < n; i++)
    data[i] = (float)i;
}

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

    fill_with_ramp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(1.0f);

    GOSoundResample::PtrFrameVector<float, float, N_PLANAR_PERF_CHANNELS> fV(
      src.data());
    const std::vector<unsigned> increments(
      nOutFrames, resamplingPos.GetFractionIncrement());

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

    fill_with_ramp(src);

    GOSoundResample::ResamplingPosition resamplingPos;

    resamplingPos.Init(1.0f);

    GOSoundResample::PtrFrameVector<float, float, N_PLANAR_PERF_CHANNELS> fV(
      src.data());
    const std::vector<unsigned> increments(
      nOutFrames, resamplingPos.GetFractionIncrement());

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

  TestPerfResampleBlockVariableRatePlanarLinear();
  TestPerfResampleBlockVariableRatePlanarPolyphase();

  std::cout << "\n========== Performance Tests Completed ==========\n";

  ReportFailedTests();
}
