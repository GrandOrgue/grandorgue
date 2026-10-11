/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundLinearFaderCore.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "sound/buffer/GOSoundBufferPlanar.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/dsp-kernels/GOSoundLinearFaderCore.h"

#include "GOTestScope.h"

const std::string GOTestPerfSoundLinearFaderCore::TEST_NAME
  = "GOTestPerfSoundLinearFaderCore";

using Core = GOSoundLinearFaderCore;

// Calibration rule: worst of the local runs and of the CI runs on the fork and
// on GrandOrgue/grandorgue (GitHub runners: AMD EPYC 7763, 9V74, 9V45, Intel
// Xeon Platinum 8573C),
// minus 10%, rounded down to two significant digits, separately for Release
// and Debug; a baseline is only ever lowered by it. Mitems/sec: the frames of
// an iteration * the channels. CI runners are noisy: sizes 512 and 2048 were
// seen there at a fraction of the local throughput. Format: {buffer_size,
// min_MItems_per_second}.
// Rebaselined 2026-10-31 against the CI runs of both repos from 2026-10-17..31
// (all after commit 341191074 changed the iteration count): every entry is
// -10% of the worst throughput observed in that window, floored to 2
// significant figures; entries above that value were left unchanged.
// fade() computes the gain of every frame by the formula start + delta *
// frameI: in the Release build it is 1.4..4 times faster than accumulating it
// (gain += delta); in the Debug build it is up to 25% slower.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_FADE_STEREO_INSIDE[] = {
#ifdef NDEBUG
  {32, 2200},
  {128, 2900},
  {512, 570},
  {2048, 490},
#else
  {32, 630},
  {128, 750},
  {512, 340},
  {2048, 340},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_FADE_STEREO_HALF[] = {
#ifdef NDEBUG
  {32, 2500},
  {128, 3900},
  {512, 590},
  {2048, 500},
#else
  {32, 790},
  {128, 910},
  {512, 380},
  {2048, 380},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_FADE_STEREO_PAST[] = {
#ifdef NDEBUG
  {32, 3700},
  {128, 4800},
  {512, 620},
  {2048, 520},
#else
  {32, 1000},
  {128, 1200},
  {512, 450},
  {2048, 420},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_FADE_STEREO_CONST_ONE[]
  = {
#ifdef NDEBUG
    {32, 5000},
    {128, 9200},
    {512, 650},
    {2048, 540},
#else
    {32, 3200},
    {128, 9900},
    {512, 550},
    {2048, 600},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_FADE_STEREO_CONST_ZERO[]
  = {
#ifdef NDEBUG
    {32, 3100},
    {128, 7400}, // a local run 8266
    {512, 590},
    {2048, 530},
#else
    {32, 1300},
    {128, 1500},
    {512, 470},
    {2048, 490},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_FADE_MONO[] = {
#ifdef NDEBUG
  {32, 1500},
  {128, 1700},
  {512, 2200},
  {2048, 470},
#else
  {32, 450},
  {128, 560},
  {512, 590},
  {2048, 310},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_FADE_PLANAR_MONO_INSIDE[]
  = {
#ifdef NDEBUG
    {32, 1300},
    {128, 1700},
    {512, 2200},
    {2048, 470},
#else
    {32, 440},
    {128, 550},
    {512, 590},
    {2048, 310},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_FADE_PLANAR_STEREO_INSIDE[]
  = {
#ifdef NDEBUG
    {32, 1500},
    {128, 2100},
    {512, 520},
    {2048, 460},
#else
    {32, 490},
    {128, 550},
    {512, 300},
    {2048, 310},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_FADE_PLANAR_STEREO_HALF[]
  = {
#ifdef NDEBUG
    {32, 1700},
    {128, 3000},
    {512, 580},
    {2048, 490},
#else
    {32, 630},
    {128, 790},
    {512, 350},
    {2048, 390},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_FADE_PLANAR_STEREO_PAST[]
  = {
#ifdef NDEBUG
    {32, 2400},
    {128, 4700},
    {512, 630},
    {2048, 520},
#else
    {32, 770},
    {128, 1000},
    {512, 440},
    {2048, 460},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_CROSSFADE_INTERLEAVED_INSIDE[]
  = {
#ifdef NDEBUG
    {32, 1400},
    {128, 1900},
    {512, 390},
    {2048, 490},
#else
    {32, 580},
    {128, 710},
    {512, 330},
    {2048, 340},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_CROSSFADE_INTERLEAVED_HALF[]
  = {
#ifdef NDEBUG
    {32, 2100},
    {128, 3100},
    {512, 410},
    {2048, 530},
#else
    {32, 950},
    {128, 1200},
    {512, 400},
    {2048, 420},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_CROSSFADE_INTERLEAVED_TO_PAST[]
  = {
#ifdef NDEBUG
    {32, 2200},
    {128, 7400},
    {512, 440},
    {2048, 570},
#else
    {32, 2000},
    {128, 6200},
    {512, 510},
    {2048, 570},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_CROSSFADE_INTERLEAVED_FROM_PAST[]
  = {
#ifdef NDEBUG
    {32, 4100},
    {128, 11000},
    {512, 660},
    {2048, 600},
#else
    {32, 3300},
    {128, 10000},
    {512, 530},
    {2048, 600},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_CROSSFADE_MONO_INSIDE[]
  = {
#ifdef NDEBUG
    {32, 1400},
    {128, 1600},
    {512, 2100},
    {2048, 450},
#else
    {32, 280},
    {128, 390},
    {512, 420},
    {2048, 300},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_CROSSFADE_STEREO_INSIDE[]
  = {
#ifdef NDEBUG
    {32, 1400},
    {128, 1900},
    {512, 530},
    {2048, 490},
#else
    {32, 320},
    {128, 370},
    {512, 300},
    {2048, 280},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_CROSSFADE_TO_PAST[] = {
#ifdef NDEBUG
  {32, 2000},
  {128, 5600},
  {512, 630},
  {2048, 570},
#else
  {32, 1300},
  {128, 4400},
  {512, 550},
  {2048, 590},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_CROSSFADE_FROM_PAST[]
  = {
#ifdef NDEBUG
    {32, 3600},
    {128, 10000},
    {512, 660},
    {2048, 600},
#else
    {32, 1800},
    {128, 6600},
    {512, 560},
    {2048, 620},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_CROSSFADE_HALF[] = {
#ifdef NDEBUG
  {32, 1700},
  {128, 2800},
  {512, 580},
  {2048, 530},
#else
  {32, 570},
  {128, 720},
  {512, 390},
  {2048, 380},
#endif
};

/**
 * Runs the fade<nChannels> benchmark.
 *
 * @param evaluate RunAndEvaluateTest of the test
 * @param name the case name
 * @param baselines the baselines
 * @param makeRamp builds the ramp for the buffer size
 * @param getOffset the dataOffsetInRamp for the buffer size
 */
template <
  unsigned nChannels,
  unsigned N,
  class EvaluateT,
  class MakeRampT,
  class OffsetT>
static void run_fade(
  EvaluateT evaluate,
  const std::string &name,
  const GOTestPerfSoundBufferBaseline (&baselines)[N],
  MakeRampT makeRamp,
  OffsetT getOffset) {
  for (const GOTestPerfSoundBufferBaseline &baseline : baselines) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> ones(nFrames * nChannels, 1.0f);
    std::vector<float> data(ones);
    const Core::GainRamp ramp = makeRamp(nFrames);
    const unsigned offset = getOffset(nFrames);

    evaluate(
      name,
      baseline,
      [&]() {
        std::copy(ones.begin(), ones.end(), data.begin());
        Core::fade<nChannels>(ramp, offset, nFrames, data.data());
      },
      nChannels,
      true);
  }
}

void GOTestPerfSoundLinearFaderCore::TestPerfFadeStereoInside() {
  std::cout << "\nPerformance test: fade<2>, inside the ramp\n";
  run_fade<2>(
    [this](auto &&...args) { RunAndEvaluateTest(args...); },
    "FadeStereoInside",
    BASELINE_FADE_STEREO_INSIDE,
    [](unsigned n) { return Core::GainRamp(0.0f, 1.0f, n); },
    [](unsigned) { return 0u; });
}

void GOTestPerfSoundLinearFaderCore::TestPerfFadeStereoHalf() {
  std::cout << "\nPerformance test: fade<2>, half inside, half past the end\n";
  run_fade<2>(
    [this](auto &&...args) { RunAndEvaluateTest(args...); },
    "FadeStereoHalf",
    BASELINE_FADE_STEREO_HALF,
    [](unsigned n) { return Core::GainRamp(0.0f, 0.5f, n); },
    [](unsigned n) { return n / 2; });
}

void GOTestPerfSoundLinearFaderCore::TestPerfFadeStereoPast() {
  std::cout << "\nPerformance test: fade<2>, past the ramp end\n";
  run_fade<2>(
    [this](auto &&...args) { RunAndEvaluateTest(args...); },
    "FadeStereoPast",
    BASELINE_FADE_STEREO_PAST,
    [](unsigned n) { return Core::GainRamp(0.0f, 0.5f, n); },
    [](unsigned n) { return n; });
}

void GOTestPerfSoundLinearFaderCore::TestPerfFadeStereoConstOne() {
  std::cout << "\nPerformance test: fade<2>, constant gain 1\n";
  run_fade<2>(
    [this](auto &&...args) { RunAndEvaluateTest(args...); },
    "FadeStereoConstOne",
    BASELINE_FADE_STEREO_CONST_ONE,
    [](unsigned) { return Core::GainRamp(1.0f, 1.0f, 0); },
    [](unsigned) { return 0u; });
}

void GOTestPerfSoundLinearFaderCore::TestPerfFadeStereoConstZero() {
  std::cout << "\nPerformance test: fade<2>, constant gain 0\n";
  run_fade<2>(
    [this](auto &&...args) { RunAndEvaluateTest(args...); },
    "FadeStereoConstZero",
    BASELINE_FADE_STEREO_CONST_ZERO,
    [](unsigned) { return Core::GainRamp(0.0f, 0.0f, 0); },
    [](unsigned) { return 0u; });
}

void GOTestPerfSoundLinearFaderCore::TestPerfFadeMono() {
  std::cout << "\nPerformance test: fade<1>, inside the ramp\n";
  run_fade<1>(
    [this](auto &&...args) { RunAndEvaluateTest(args...); },
    "FadeMono",
    BASELINE_FADE_MONO,
    [](unsigned n) { return Core::GainRamp(0.0f, 1.0f, n); },
    [](unsigned) { return 0u; });
}

/**
 * Runs the fadePlanar benchmark.
 *
 * @param evaluate RunAndEvaluateTest of the test
 * @param name the case name
 * @param baselines the baselines
 * @param nChannels number of planar channels
 * @param makeRamp builds the ramp for the buffer size
 * @param getOffset the dataOffsetInRamp for the buffer size
 */
template <unsigned N, class EvaluateT, class MakeRampT, class OffsetT>
static void run_fade_planar(
  EvaluateT evaluate,
  const std::string &name,
  const GOTestPerfSoundBufferBaseline (&baselines)[N],
  unsigned nChannels,
  MakeRampT makeRamp,
  OffsetT getOffset) {
  for (const GOTestPerfSoundBufferBaseline &baseline : baselines) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> ones(nFrames * nChannels, 1.0f);
    std::vector<float> data(ones);
    GOSoundBufferPlanarMutable inOutBuffer(data.data(), nChannels, nFrames);
    const Core::GainRamp ramp = makeRamp(nFrames);
    const unsigned offset = getOffset(nFrames);

    evaluate(
      name,
      baseline,
      [&]() {
        std::copy(ones.begin(), ones.end(), data.begin());
        Core::fadePlanar(ramp, offset, inOutBuffer);
      },
      nChannels,
      true);
  }
}

void GOTestPerfSoundLinearFaderCore::TestPerfFadePlanar() {
  std::cout << "\nPerformance test: fadePlanar\n";
  auto evaluate = [this](auto &&...args) { RunAndEvaluateTest(args...); };

  run_fade_planar(
    evaluate,
    "FadePlanarMonoInside",
    BASELINE_FADE_PLANAR_MONO_INSIDE,
    1,
    [](unsigned n) { return Core::GainRamp(0.0f, 1.0f, n); },
    [](unsigned) { return 0u; });
  run_fade_planar(
    evaluate,
    "FadePlanarStereoInside",
    BASELINE_FADE_PLANAR_STEREO_INSIDE,
    2,
    [](unsigned n) { return Core::GainRamp(0.0f, 1.0f, n); },
    [](unsigned) { return 0u; });
  run_fade_planar(
    evaluate,
    "FadePlanarStereoHalf",
    BASELINE_FADE_PLANAR_STEREO_HALF,
    2,
    [](unsigned n) { return Core::GainRamp(0.0f, 0.5f, n); },
    [](unsigned n) { return n / 2; });
  run_fade_planar(
    evaluate,
    "FadePlanarStereoPast",
    BASELINE_FADE_PLANAR_STEREO_PAST,
    2,
    [](unsigned n) { return Core::GainRamp(0.0f, 0.5f, n); },
    [](unsigned n) { return n; });
}

/**
 * Runs the crossfade<2> benchmark on interleaved stereo.
 *
 * @param evaluate RunAndEvaluateTest of the test
 * @param name the case name
 * @param baselines the baselines
 * @param direction the crossfade direction
 * @param getRampFrames the ramp length for the buffer size
 * @param getOffset the dataOffsetInRamp for the buffer size
 */
template <unsigned N, class EvaluateT>
static void run_crossfade_interleaved(
  EvaluateT evaluate,
  const std::string &name,
  const GOTestPerfSoundBufferBaseline (&baselines)[N],
  Core::CrossfadeDirection direction,
  unsigned (*getRampFrames)(unsigned),
  unsigned (*getOffset)(unsigned)) {
  for (const GOTestPerfSoundBufferBaseline &baseline : baselines) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> other(nFrames * 2, 1.0f);
    const std::vector<float> zeros(nFrames * 2, 0.0f);
    std::vector<float> data(zeros);
    const Core::CrossfadeRamp ramp(direction, getRampFrames(nFrames));
    const unsigned offset = getOffset(nFrames);

    evaluate(
      name,
      baseline,
      [&]() {
        std::copy(zeros.begin(), zeros.end(), data.begin());
        Core::crossfade<2>(ramp, offset, nFrames, other.data(), data.data());
      },
      2,
      true);
  }
}

void GOTestPerfSoundLinearFaderCore::TestPerfCrossfadeInterleaved() {
  std::cout << "\nPerformance test: crossfade<2> on interleaved stereo\n";
  auto evaluate = [this](auto &&...args) { RunAndEvaluateTest(args...); };

  run_crossfade_interleaved(
    evaluate,
    "CrossfadeInterleavedInside",
    BASELINE_CROSSFADE_INTERLEAVED_INSIDE,
    Core::TO_OTHER,
    [](unsigned n) { return n; },
    [](unsigned) { return 0u; });
  run_crossfade_interleaved(
    evaluate,
    "CrossfadeInterleavedHalf",
    BASELINE_CROSSFADE_INTERLEAVED_HALF,
    Core::TO_OTHER,
    [](unsigned n) { return n; },
    [](unsigned n) { return n / 2; });
  run_crossfade_interleaved(
    evaluate,
    "CrossfadeInterleavedToOtherPast",
    BASELINE_CROSSFADE_INTERLEAVED_TO_PAST,
    Core::TO_OTHER,
    [](unsigned n) { return n; },
    [](unsigned n) { return n; });
  run_crossfade_interleaved(
    evaluate,
    "CrossfadeInterleavedFromOtherPast",
    BASELINE_CROSSFADE_INTERLEAVED_FROM_PAST,
    Core::FROM_OTHER,
    [](unsigned n) { return n; },
    [](unsigned n) { return n; });
}

/**
 * Runs the crossfadePlanar benchmark.
 *
 * @param evaluate RunAndEvaluateTest of the test
 * @param name the case name
 * @param baselines the baselines
 * @param nChannels number of planar channels
 * @param direction the crossfade direction
 * @param getRampFrames the ramp length for the buffer size
 * @param getOffset the dataOffsetInRamp for the buffer size
 */
template <unsigned N, class EvaluateT>
static void run_crossfade(
  EvaluateT evaluate,
  const std::string &name,
  const GOTestPerfSoundBufferBaseline (&baselines)[N],
  unsigned nChannels,
  Core::CrossfadeDirection direction,
  unsigned (*getRampFrames)(unsigned),
  unsigned (*getOffset)(unsigned)) {
  for (const GOTestPerfSoundBufferBaseline &baseline : baselines) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> other(nFrames * nChannels, 1.0f);
    const std::vector<float> ones(nFrames * nChannels, 0.0f);
    std::vector<float> data(ones);
    const GOSoundBufferPlanar otherBuffer(other.data(), nChannels, nFrames);
    GOSoundBufferPlanarMutable inOutBuffer(data.data(), nChannels, nFrames);
    const Core::CrossfadeRamp ramp(direction, getRampFrames(nFrames));
    const unsigned offset = getOffset(nFrames);

    evaluate(
      name,
      baseline,
      [&]() {
        std::copy(ones.begin(), ones.end(), data.begin());
        Core::crossfadePlanar(ramp, offset, otherBuffer, inOutBuffer);
      },
      nChannels,
      true);
  }
}

void GOTestPerfSoundLinearFaderCore::TestPerfCrossfadePlanarInside() {
  std::cout << "\nPerformance test: crossfadePlanar, inside the ramp\n";
  auto evaluate = [this](auto &&...args) { RunAndEvaluateTest(args...); };

  run_crossfade(
    evaluate,
    "CrossfadeMonoInside",
    BASELINE_CROSSFADE_MONO_INSIDE,
    1,
    Core::TO_OTHER,
    [](unsigned n) { return n; },
    [](unsigned) { return 0u; });
  run_crossfade(
    evaluate,
    "CrossfadeStereoInside",
    BASELINE_CROSSFADE_STEREO_INSIDE,
    2,
    Core::TO_OTHER,
    [](unsigned n) { return n; },
    [](unsigned) { return 0u; });
}

void GOTestPerfSoundLinearFaderCore::TestPerfCrossfadePlanarPast() {
  std::cout << "\nPerformance test: crossfadePlanar, past the ramp end\n";
  auto evaluate = [this](auto &&...args) { RunAndEvaluateTest(args...); };

  run_crossfade(
    evaluate,
    "CrossfadeToOtherPast",
    BASELINE_CROSSFADE_TO_PAST,
    2,
    Core::TO_OTHER,
    [](unsigned n) { return n; },
    [](unsigned n) { return n; });
  run_crossfade(
    evaluate,
    "CrossfadeFromOtherPast",
    BASELINE_CROSSFADE_FROM_PAST,
    2,
    Core::FROM_OTHER,
    [](unsigned n) { return n; },
    [](unsigned n) { return n; });
}

void GOTestPerfSoundLinearFaderCore::TestPerfCrossfadePlanarHalf() {
  std::cout << "\nPerformance test: crossfadePlanar, half inside, half past\n";
  run_crossfade(
    [this](auto &&...args) { RunAndEvaluateTest(args...); },
    "CrossfadeHalf",
    BASELINE_CROSSFADE_HALF,
    2,
    Core::TO_OTHER,
    [](unsigned n) { return n; },
    [](unsigned n) { return n / 2; });
}

bool GOTestPerfSoundLinearFaderCore::setUp() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundLinearFaderCore "
               "==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << GetNFrames() << " frames per measurement\n";
  return true;
}

void GOTestPerfSoundLinearFaderCore::run() {
  GO_RUN_TEST(TestPerfFadeStereoInside())
  GO_RUN_TEST(TestPerfFadeStereoHalf())
  GO_RUN_TEST(TestPerfFadeStereoPast())
  GO_RUN_TEST(TestPerfFadeStereoConstOne())
  GO_RUN_TEST(TestPerfFadeStereoConstZero())
  GO_RUN_TEST(TestPerfFadeMono())
  GO_RUN_TEST(TestPerfFadePlanar())
  GO_RUN_TEST(TestPerfCrossfadeInterleaved())
  GO_RUN_TEST(TestPerfCrossfadePlanarInside())
  GO_RUN_TEST(TestPerfCrossfadePlanarPast())
  GO_RUN_TEST(TestPerfCrossfadePlanarHalf())
  // It must stay here and not in tearDown(), because an exception thrown from
  // tearDown() is not counted as a test failure. Does nothing if no test was
  // run
  ReportFailedTests();
}

bool GOTestPerfSoundLinearFaderCore::tearDown() {
  std::cout << "\n========== Performance Tests Completed ==========\n";
  return true;
}
