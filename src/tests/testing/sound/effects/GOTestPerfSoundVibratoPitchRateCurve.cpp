/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundVibratoPitchRateCurve.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/effects/GOSoundVibratoPitchRateCurve.h"

#include "../GOSoundTestSignals.h"
#include "GOTestScope.h"

const std::string GOTestPerfSoundVibratoPitchRateCurve::TEST_NAME
  = "GOTestPerfSoundVibratoPitchRateCurve";

using Curve = GOSoundVibratoPitchRateCurve;

static constexpr unsigned UNIT_RATE = GOSoundResample::UPSAMPLE_FACTOR;
static constexpr unsigned N_FRAMES_PER_PROCESS = 128;

/* The unit is Mcells/s of the curve (the curve length is the "size"), for the
 * cursor Mperiods/s (the size is 1). Calibration rule throughout this file:
 * worst observed run, minus 10%, floored to 2 significant digits - from local
 * runs and from the CI runs on the fork (Release and Debug); a baseline is
 * only ever lowered by it. */
static constexpr GOTestPerfSoundBufferBaseline BASELINE_APPLY_AFFINE[] = {
#ifdef NDEBUG
  // Release, worst of 3 runs 161.1/158.1/158.9 Mcells/s, minus 10%
  {100, 140},
  {10000, 140},
  {100000, 140},
  {1000000, 130}, // worst of 3 runs 155.2
#else
  // Debug, worst of 3 runs 116.7/153.5/140.9 Mcells/s, minus 10%
  {100, 100},
  {10000, 130},
  {100000, 120},
  {1000000, 130}, // worst of 3 runs 146.2
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_NORMALISE_SHIFT[] = {
#ifdef NDEBUG
  // Release, worst of the local and CI runs 873.4/839.1/834.9/800.8/738.1
  // Mcells/s, minus 10%, floored to 2 significant figures (rebaselined
  // 2026-10-31: a run on an Intel Xeon Platinum 8573C was 15..20% below all
  // the earlier runs)
  {100, 780},
  {1000, 750},
  {10000, 750},
  {100000, 720},
  {1000000, 660},
#else
  // Debug, worst of local and CI runs 288.0/444.2/512.0/384.0/448.0 Mcells/s,
  // minus 10%
  {100, 250},
  {1000, 340},
  {10000, 360},
  {100000, 340},
  {1000000, 350},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_NORMALISE_PRESSED[] = {
#ifdef NDEBUG
  // Release, worst of 3 runs 136.3/142.9/141.2/139.2 Mcells/s, minus 10%
  {100, 120},
  {1000, 120},
  {10000, 120},
  {100000, 120},
  {1000000, 120}, // worst of 3 runs 137.5
#else
  // Debug, worst of 6 runs 113.1/121.0/122.6/127.6 Mcells/s, minus 10%
  {100, 100},
  {1000, 100},
  {10000, 110},
  {100000, 110},
  {1000000, 110}, // worst of 3 runs 124.3
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_BUILD_NOT_NORMALISED[]
  = {
#ifdef NDEBUG
    // Release, worst of local and CI runs 184.0/133.6/175.1/166.3 Mcells/s,
    // minus 10%
    {100, 160},
    {1000, 120},
    {10000, 150},
    {100000, 140},
    {1000000, 130}, // worst of 3 runs 153.0
#else
    // Debug, worst of local and CI runs 93.0/129.4/118.1/118.0 Mcells/s, minus
    // 10%
    {100, 83},
    {1000, 110},
    {10000, 100},
    {100000, 100},
    {1000000, 97}, // worst of local and CI runs 107.8
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_BUILD_NORMALISED[] = {
#ifdef NDEBUG
  // Release, worst of 3 runs 142.7/156.3/154.2/133.3 Mcells/s, minus 10%
  {100, 120},
  {1000, 140},
  {10000, 130},
  {100000, 120},
  {1000000, 110}, // worst of 3 runs 126.3
#else
  // Debug, worst of local and CI runs 88.1/89.1/92.0/92.1 Mcells/s, minus 10%
  {100, 64},
  {1000, 80},
  {10000, 82},
  {100000, 82},
  {1000000, 85}, // worst of 3 runs 94.7
#endif
};

/* One period per call; the harness prints the unit as Mframes/sec, it is
 * Mperiods/s here. The per-case baselines are in BASELINE_POSITION_PERIODS,
 * in the order of the loops in TestPerfChunkDescriptionAdvance(). */
static constexpr double BASELINE_POSITION_PERIODS[] = {
#ifdef NDEBUG
  // Release, worst of local and CI runs
  // 274.7/291.6/300.7/263.8/228.5/243.5/137.7/231.3 Mperiods/s, minus 10%
  240,
  260,
  270,
  230,
  200,
  210,
  120,
  200,
#else
  // Debug, worst of local and CI runs
  // 97.5/98.0/97.4/97.9/98.3/101.4/94.8/100.3 Mperiods/s, minus 10%; the last
  // two cases lowered after local runs 86.6 (512/100000) and 84.4
  // (2048/100000), minus 10%
  87,
  88,
  87,
  88,
  88,
  77,
  85,
  76,
#endif
};

/** About the same total work (in cells) for every curve length: returns the
 * number of cells to process, a multiple of curveLen */
static unsigned get_n_frames(unsigned curveLen) {
  return std::clamp(5000000u / curveLen, 5u, 50000u) * curveLen;
}

std::vector<uint16_t> GOTestPerfSoundVibratoPitchRateCurve::makeMasterRates(
  unsigned curveLen, const std::vector<float> &deviations) {
  Curve curve;

  curve.BuildInternal(deviations.data(), curveLen, N_FRAMES_PER_PROCESS, false);

  const uint16_t *pRates
    = curve.GetChunk(Curve::LoopPosition(curve.GetNLoopFrames()));

  return std::vector<uint16_t>(pRates, pRates + curveLen);
}

void GOTestPerfSoundVibratoPitchRateCurve::TestPerfApplyAffineTransform() {
  std::cout << "\nPerformance test: applyAffineTransform (k = 1/2)\n";
  for (const GOTestPerfSoundBufferBaseline &baseline : BASELINE_APPLY_AFFINE) {
    const unsigned len = baseline.m_BufferSize;
    const std::vector<uint16_t> master = makeMasterRates(
      len,
      GOSoundTestSignals::makeSinePeriod(
        len, GOSoundTestSignals::centsToDeviation(100.0f), 1000.0f));
    std::vector<uint16_t> work(len);

    m_NFrames = get_n_frames(len);
    RunAndEvaluateTest("ApplyAffineTransform", baseline, [&]() {
      std::copy(master.begin(), master.end(), work.begin());
      Curve::applyAffineTransform(
        work.data(), len, Curve::AffineCoefficients{1, 2, UNIT_RATE / 2});
    });
  }
}

void GOTestPerfSoundVibratoPitchRateCurve::TestPerfNormaliseShift() {
  std::cout << "\nPerformance test: normalisePitchRateCurve (plain shift)\n";
  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_NORMALISE_SHIFT) {
    const unsigned len = baseline.m_BufferSize;
    const std::vector<uint16_t> master = makeMasterRates(
      len,
      GOSoundTestSignals::makeSinePeriod(
        len, GOSoundTestSignals::centsToDeviation(100.0f), 1000.0f));
    const uint64_t masterSum
      = std::accumulate(master.begin(), master.end(), (uint64_t)0);
    // A non-zero remainder, so that both final passes work
    const uint64_t target = (uint64_t)len * UNIT_RATE + len / 2;
    std::vector<uint16_t> work(len);

    m_NFrames = get_n_frames(len);
    RunAndEvaluateTest("NormaliseShift", baseline, [&]() {
      std::copy(master.begin(), master.end(), work.begin());
      Curve::normalisePitchRateCurve(work.data(), len, masterSum, target);
    });
  }
}

void GOTestPerfSoundVibratoPitchRateCurve::TestPerfNormalisePressed() {
  std::cout << "\nPerformance test: normalisePitchRateCurve (pressing)\n";
  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_NORMALISE_PRESSED) {
    const unsigned len = baseline.m_BufferSize;
    // A square wave between the bounds of the range: a plain shift does not
    // fit
    std::vector<float> deviations(len);

    for (unsigned frameI = 0; frameI < len; ++frameI)
      deviations[frameI]
        = (frameI / 10) % 2 ? Curve::MAX_DEVIATION : Curve::MIN_DEVIATION;

    const std::vector<uint16_t> master = makeMasterRates(len, deviations);
    const uint64_t masterSum
      = std::accumulate(master.begin(), master.end(), (uint64_t)0);
    const uint64_t target = (uint64_t)len * UNIT_RATE;
    std::vector<uint16_t> work(len);

    m_NFrames = get_n_frames(len);
    RunAndEvaluateTest("NormalisePressed", baseline, [&]() {
      std::copy(master.begin(), master.end(), work.begin());
      Curve::normalisePitchRateCurve(work.data(), len, masterSum, target);
    });
  }
}

void GOTestPerfSoundVibratoPitchRateCurve::TestPerfBuildNotNormalised() {
  std::cout << "\nPerformance test: BuildInternal (not normalised)\n";
  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_BUILD_NOT_NORMALISED) {
    const unsigned len = baseline.m_BufferSize;
    const std::vector<float> deviations = GOSoundTestSignals::makeSinePeriod(
      len, GOSoundTestSignals::centsToDeviation(100.0f), 1000.0f);
    Curve curve;

    m_NFrames = get_n_frames(len);
    RunAndEvaluateTest("BuildNotNormalised", baseline, [&]() {
      curve.BuildInternal(deviations.data(), len, N_FRAMES_PER_PROCESS, false);
      // The curve must be empty for the next run
      curve.Destroy();
    });
  }
}

void GOTestPerfSoundVibratoPitchRateCurve::TestPerfBuildNormalised() {
  std::cout << "\nPerformance test: Build (normalised)\n";
  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_BUILD_NORMALISED) {
    const unsigned len = baseline.m_BufferSize;
    const std::vector<float> deviations = GOSoundTestSignals::makeSinePeriod(
      len, GOSoundTestSignals::centsToDeviation(100.0f), 1000.0f);
    Curve curve;

    m_NFrames = get_n_frames(len);
    RunAndEvaluateTest("BuildNormalised", baseline, [&]() {
      curve.Build(deviations.data(), len, N_FRAMES_PER_PROCESS);
      // The curve must be empty for the next run
      curve.Destroy();
    });
  }
}

void GOTestPerfSoundVibratoPitchRateCurve::TestPerfCopy() {
  std::cout << "\nPerformance test: copy of the master rates (reference)\n";
  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_NORMALISE_SHIFT) {
    const unsigned len = baseline.m_BufferSize;
    const std::vector<uint16_t> master = makeMasterRates(
      len,
      GOSoundTestSignals::makeSinePeriod(
        len, GOSoundTestSignals::centsToDeviation(100.0f), 1000.0f));
    std::vector<uint16_t> work(len);
    // The copy has no baseline of its own
    const GOTestPerfSoundBufferBaseline noBaseline = {len, 0.0};

    m_NFrames = get_n_frames(len);
    RunAndEvaluateTest("Copy", noBaseline, [&]() {
      std::copy(master.begin(), master.end(), work.begin());
    });
  }
}

void GOTestPerfSoundVibratoPitchRateCurve::TestPerfChunkDescriptionAdvance() {
  std::cout << "\nPerformance test: GetChunkDescription + Advance\n";
  unsigned caseI = 0;

  for (const unsigned nFrames : {32u, 128u, 512u, 2048u})
    for (const unsigned loopLen : {1000u, 100000u}) {
      const std::vector<float> deviations = GOSoundTestSignals::makeSinePeriod(
        loopLen, GOSoundTestSignals::centsToDeviation(100.0f), 1000.0f);
      Curve curve;
      Curve::LoopPosition pos(loopLen);
      unsigned long checksum = 0;

      curve.Build(deviations.data(), loopLen, nFrames);
      m_NFrames = 10000000; // the baseline buffer size is 1
      RunAndEvaluateTest(
        "PositionPeriod" + std::to_string(nFrames) + "/"
          + std::to_string(loopLen),
        GOTestPerfSoundBufferBaseline{1, BASELINE_POSITION_PERIODS[caseI++]},
        [&]() {
          // The sum is used so that the optimiser cannot drop the call
          checksum += curve.GetChunkDescription(pos).nTotalUnits;
          pos.Advance(nFrames);
        });
      if (checksum == 1)
        std::cout << "";
    }
}

bool GOTestPerfSoundVibratoPitchRateCurve::setUp() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for "
               "GOSoundVibratoPitchRateCurve ==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  return true;
}

void GOTestPerfSoundVibratoPitchRateCurve::run() {
  GO_RUN_TEST(TestPerfApplyAffineTransform())
  GO_RUN_TEST(TestPerfNormaliseShift())
  GO_RUN_TEST(TestPerfNormalisePressed())
  GO_RUN_TEST(TestPerfBuildNotNormalised())
  GO_RUN_TEST(TestPerfBuildNormalised())
  GO_RUN_TEST(TestPerfCopy())
  GO_RUN_TEST(TestPerfChunkDescriptionAdvance())
  // It must stay here and not in tearDown(), because an exception thrown from
  // tearDown() is not counted as a test failure. Does nothing if no test was
  // run
  ReportFailedTests();
}

bool GOTestPerfSoundVibratoPitchRateCurve::tearDown() {
  std::cout << "\n========== Performance Tests Completed ==========\n";
  return true;
}
