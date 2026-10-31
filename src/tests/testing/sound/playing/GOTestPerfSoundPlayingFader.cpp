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
// Rebaselined 2026-10-31 against the CI runs of both repos from 2026-10-17..31
// (all after commit 341191074 changed the iteration count): every entry is
// -10% of the worst throughput observed in that window, floored to 2
// significant figures; entries above that value were left unchanged.
// Every iteration refills the buffer with ones first (a repeated in-place
// fade would drive the data to denormals); the refill is a part of the
// measured time, equal before and after the refactoring of the fader.
// Format: {buffer_size, min_MItems_per_second}.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_CONSTANT_GAIN[] = {
#ifdef NDEBUG
  // Release, worst runs 3433.2/5520.4/492.2/589.3, minus 10%
  {32, 3000},
  {128, 4900},
  {512, 440},
  {2048, 530},
#else
  // Debug, worst runs 1497.5/2221.5/484/501.5, minus 10%
  {32, 1300},
  {128, 1900},
  {512, 430},
  {2048, 450},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_RAMP[] = {
#ifdef NDEBUG
  // Release, worst runs 2737.0/2589.6/457.0/541.9, minus 10%
  {32, 2400},
  {128, 2300},
  {512, 410},
  {2048, 480},
#else
  // Debug, worst runs 1104.3/1289.3/449/474.7, minus 10%
  {32, 990},
  {128, 1100},
  {512, 400},
  {2048, 420},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_GROWTH_TO_DECAY[] = {
#ifdef NDEBUG
  // Release, worst runs 3049.4/4018.5/874.7/1037.0, minus 10%
  {32, 2700},
  {128, 3600},
  {512, 780},
  {2048, 930},
#else
  // Debug, worst runs 1401/1818.8/783/811.9, minus 10%
  {32, 1200},
  {128, 1600},
  {512, 700},
  {2048, 730},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_TARGET_REACHED_MID_BLOCK[]
  = {
#ifdef NDEBUG
    // Release, worst runs 2880/2610.0/457.8/542.5, minus 10%
    {32, 2500},
    {128, 2300},
    {512, 410},
    {2048, 480},
#else
    // Debug, worst runs 1203.4/1312.1/450/476.1, minus 10%
    {32, 1000},
    {128, 1100},
    {512, 400},
    {2048, 420},
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
