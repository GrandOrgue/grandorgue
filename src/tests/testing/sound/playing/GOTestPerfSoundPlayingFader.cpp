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
  // Release, worst of the runs after the refactoring onto
  // GOSoundLinearFaderCore, minus 10%
  {32, 3000},
  {128, 5300},
  {512, 520},
  {2048, 520},
#else
  // Debug, worst of the runs after the refactoring onto
  // GOSoundLinearFaderCore, minus 10%
  {32, 1200},
  {128, 1500},
  {512, 380},
  {2048, 490},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_RAMP[] = {
#ifdef NDEBUG
  // Release, worst of the runs after the refactoring onto
  // GOSoundLinearFaderCore, minus 10%: the gain of every frame is computed by
  // the formula, slower than the former accumulation (on the same CI CPU:
  // -32% on 32 frames, -12% on 128, -3..-5% on 512 and 2048)
  {32, 1400},
  {128, 2000},
  {512, 460},
  {2048, 460},
#else
  // Debug, worst of the runs after the refactoring onto
  // GOSoundLinearFaderCore, minus 10% (the unoptimised Debug build is
  // slower, -20..-50% on the same CI CPU)
  {32, 610},
  {128, 720},
  {512, 300},
  {2048, 360},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_GROWTH_TO_DECAY[] = {
#ifdef NDEBUG
  // Release, worst of the runs after the refactoring onto
  // GOSoundLinearFaderCore, minus 10%
  {32, 2000},
  {128, 3000},
  {512, 1000},
  {2048, 890},
#else
  // Debug, worst of the runs after the refactoring onto
  // GOSoundLinearFaderCore, minus 10%
  {32, 810},
  {128, 1100},
  {512, 540},
  {2048, 650},
#endif
};

static constexpr GOTestPerfSoundBufferBaseline
  BASELINE_TARGET_REACHED_MID_BLOCK[]
  = {
#ifdef NDEBUG
    // Release, worst of the runs after the refactoring onto
    // GOSoundLinearFaderCore, minus 10%
    {32, 1500},
    {128, 2000},
    {512, 540},
    {2048, 460},
#else
    // Debug, worst of the runs after the refactoring onto
    // GOSoundLinearFaderCore, minus 10%
    {32, 630},
    {128, 820},
    {512, 300},
    {2048, 370},
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
