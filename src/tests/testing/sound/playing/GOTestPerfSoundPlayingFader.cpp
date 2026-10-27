/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundPlayingFader.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundPlayingFader.h"

#include "GOTestScope.h"

const std::string GOTestPerfSoundPlayingFader::TEST_NAME
  = "GOTestPerfSoundPlayingFader";

// Calibration rule: worst of all local and CI runs (GrandOrgue/grandorgue and
// the fork), minus 10%, rounded down to two significant digits, separately
// for Release and Debug. Mitems/sec: the frames of an iteration * 2 channels *
// the Process() calls of the iteration. CI runners are noisy: sizes 512 and
// 2048 were seen there at a fraction of the local throughput.
// Every iteration refills the buffer with ones first (a repeated in-place
// fade would drive the data to denormals); the refill is a part of the
// measured time, equal before and after the refactoring of the fader.
// Format: {buffer_size, min_MItems_per_second}.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_CONSTANT_GAIN[] = {
#ifdef NDEBUG
  // Release, worst runs 4221/6816/702/668, minus 10%
  {32, 3700},
  {128, 6100},
  {512, 630},
  {2048, 600},
#else
  // Debug, worst runs 1611/2285/484/557, minus 10%
  {32, 1400},
  {128, 2000},
  {512, 430},
  {2048, 500},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_RAMP[] = {
#ifdef NDEBUG
  // Release, worst runs 2979/3182/635/605, minus 10%
  {32, 2600},
  {128, 2500}, // a CI run 2782
  {512, 570},
  {2048, 540},
#else
  // Debug, worst runs 1378/1592/449/511, minus 10%
  {32, 1200},
  {128, 1400},
  {512, 400},
  {2048, 450},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_GROWTH_TO_DECAY[] = {
#ifdef NDEBUG
  // Release, worst runs 3462/4625/1197/1114, minus 10%
  {32, 3100},
  {128, 4100},
  {512, 1000},
  {2048, 1000},
#else
  // Debug, worst runs 1401/1943/783/881, minus 10%
  {32, 1200},
  {128, 1700},
  {512, 700},
  {2048, 790},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_TARGET_REACHED_MID_BLOCK[]
  = {
#ifdef NDEBUG
    // Release, worst runs 2880/3199/637/585, minus 10%
    {32, 2500},
    {128, 2500}, // a CI run 2779
    {512, 570},
    {2048, 520},
#else
    // Debug, worst runs 1427/1619/450/514, minus 10%
    {32, 1200},
    {128, 1400},
    {512, 400},
    {2048, 460},
#endif
};

static constexpr unsigned N_CHANNELS = 2;

void GOTestPerfSoundPlayingFader::TestPerfConstantGain() {
  std::cout << "\nPerformance test: GOSoundPlayingFader::Process, "
               "constant gain\n";
  for (const GOTestPerfSoundBufferBaseline &baseline : BASELINE_CONSTANT_GAIN) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> ones(nFrames * N_CHANNELS, 1.0f);
    std::vector<float> data(ones);
    GOSoundBufferMutable buffer(data.data(), N_CHANNELS, nFrames);
    GOSoundPlayingFader fader;

    RunAndEvaluateTest(
      "FaderConstantGain",
      baseline,
      [&fader, &buffer, &data, &ones]() {
        std::copy(ones.begin(), ones.end(), data.begin());
        fader.Setup(0.5f, 1.0f);
        fader.Process(1.0f, buffer);
      },
      N_CHANNELS,
      true);
  }
}

void GOTestPerfSoundPlayingFader::TestPerfRamp() {
  std::cout << "\nPerformance test: GOSoundPlayingFader::Process, "
               "ramp over the whole block\n";
  for (const GOTestPerfSoundBufferBaseline &baseline : BASELINE_RAMP) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> ones(nFrames * N_CHANNELS, 1.0f);
    std::vector<float> data(ones);
    GOSoundBufferMutable buffer(data.data(), N_CHANNELS, nFrames);
    GOSoundPlayingFader fader;

    RunAndEvaluateTest(
      "FaderRamp",
      baseline,
      [&fader, &buffer, &data, &ones, nFrames]() {
        std::copy(ones.begin(), ones.end(), data.begin());
        fader.Setup(1.0f, 1.0f, nFrames * 4);
        fader.Process(1.0f, buffer);
      },
      N_CHANNELS,
      true);
  }
}

void GOTestPerfSoundPlayingFader::TestPerfGrowthToDecay() {
  std::cout << "\nPerformance test: GOSoundPlayingFader::Process, "
               "growth turned to decay inside the block (2 calls)\n";
  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_GROWTH_TO_DECAY) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> ones(nFrames * N_CHANNELS, 1.0f);
    std::vector<float> data(ones);
    GOSoundBufferMutable buffer(data.data(), N_CHANNELS, nFrames);
    GOSoundPlayingFader fader;

    // The first block rises 0 -> 0.5, then the decay of the same slope starts:
    // the crossing is at 0.75, in the middle of the second block
    RunAndEvaluateTest(
      "FaderGrowthToDecay",
      baseline,
      [&fader, &buffer, &data, &ones, nFrames]() {
        std::copy(ones.begin(), ones.end(), data.begin());
        fader.Setup(1.0f, 1.0f, nFrames * 2);
        fader.Process(1.0f, buffer);
        fader.StartDecreasingVolume(nFrames * 2);
        fader.Process(1.0f, buffer);
      },
      N_CHANNELS * 2,
      true);
  }
}

void GOTestPerfSoundPlayingFader::TestPerfTargetReachedMidBlock() {
  std::cout << "\nPerformance test: GOSoundPlayingFader::Process, "
               "target reached in the middle of the block\n";
  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_TARGET_REACHED_MID_BLOCK) {
    const unsigned nFrames = baseline.m_BufferSize;
    const std::vector<float> ones(nFrames * N_CHANNELS, 1.0f);
    std::vector<float> data(ones);
    GOSoundBufferMutable buffer(data.data(), N_CHANNELS, nFrames);
    GOSoundPlayingFader fader;

    RunAndEvaluateTest(
      "FaderTargetReachedMidBlock",
      baseline,
      [&fader, &buffer, &data, &ones, nFrames]() {
        std::copy(ones.begin(), ones.end(), data.begin());
        fader.Setup(1.0f, 1.0f, nFrames / 2);
        fader.Process(1.0f, buffer);
      },
      N_CHANNELS,
      true);
  }
}

bool GOTestPerfSoundPlayingFader::setUp() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundPlayingFader "
               "==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << GetNFrames() << " frames per measurement\n";
  return true;
}

void GOTestPerfSoundPlayingFader::run() {
  GO_RUN_TEST(TestPerfConstantGain())
  GO_RUN_TEST(TestPerfRamp())
  GO_RUN_TEST(TestPerfGrowthToDecay())
  GO_RUN_TEST(TestPerfTargetReachedMidBlock())
  // It must stay here and not in tearDown(), because an exception thrown from
  // tearDown() is not counted as a test failure. Does nothing if no test was
  // run
  ReportFailedTests();
}

bool GOTestPerfSoundPlayingFader::tearDown() {
  std::cout << "\n========== Performance Tests Completed ==========\n";
  return true;
}
