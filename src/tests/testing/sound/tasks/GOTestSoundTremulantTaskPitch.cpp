/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTremulantTaskPitch.h"

#include <cmath>

#include "model/GOTremulant.h"
#include "sound/dsp-kernels/GOSoundResample.h"

#include "GOSoundTremulantTaskTestFixture.h"
#include "GOTestScope.h"

const std::string GOTestSoundTremulantTaskPitch::TEST_NAME
  = "GOTestSoundTremulantTaskPitch";

using LoopPosition = GOSoundTypedBufferLoopBase::LoopPosition;
using ChunkDescription = GOSoundVibratoPitchRateCurve::ChunkDescription;

static constexpr unsigned N_FRAMES = TASK_TEST_N_FRAMES;
static constexpr unsigned N_LOOP = TASK_TEST_N_LOOP;
static constexpr int UNIT = GOSoundResample::UPSAMPLE_FACTOR;

/**
 * @return the sum of the items of the chunk
 */
static unsigned sum_of(const ChunkDescription &chunk) {
  unsigned sum = 0;

  for (unsigned frameI = 0; frameI < chunk.nFrames; ++frameI)
    sum += chunk.pRateUnits[frameI];
  return sum;
}

/**
 * Runs rounds engaged until the start ramp is over
 */
static void run_to_full_depth(GOSoundTremulantTaskTestFixture &fixture) {
  fixture.SetEngaged(true);
  for (unsigned roundI = 0; roundI < 4; ++roundI)
    fixture.RunRound();
}

void GOTestSoundTremulantTaskPitch::TestPitchChunkFullDepth() {
  GOSoundTremulantTaskTestFixture fixture(TASK_TEST_PITCH_DEVIATION);
  const GOSoundVibratoPitchRateCurve &curve
    = fixture.task.o_WaveForm->GetPitchRateCurve();

  run_to_full_depth(fixture);

  const LoopPosition pos = fixture.task.m_position;

  fixture.RunRound();

  const ChunkDescription &chunk = fixture.task.GetPitchRateChunk();

  GOAssert(
    chunk.pRateUnits == curve.GetChunk(pos), "the chunk must be the curve's");
  GOAssert(
    chunk.nTotalUnits == curve.GetChunkDescription(pos).nTotalUnits,
    "wrong sum");
  GOAssert(chunk.nFrames == N_FRAMES, "wrong chunk length");
}

void GOTestSoundTremulantTaskPitch::TestPitchChunkRamp() {
  GOSoundTremulantTaskTestFixture fixture(TASK_TEST_PITCH_DEVIATION);
  const GOSoundTremulantWaveForm &waveForm = *fixture.task.o_WaveForm;
  const auto &ramp = waveForm.GetRamp(true);
  const uint16_t *pLoopRates
    = waveForm.GetPitchRateCurve().GetChunk(LoopPosition(N_LOOP));

  fixture.SetEngaged(true);
  fixture.RunRound();

  const ChunkDescription &chunk = fixture.task.GetPitchRateChunk();

  GOAssert(chunk.HasRates(), "a ramp must publish the rates");
  GOAssert(chunk.nFrames == N_FRAMES, "wrong chunk length");
  for (unsigned frameI = 0; frameI < N_FRAMES; ++frameI) {
    const unsigned offset
      = (unsigned)std::llround(ramp.GetGainSumBefore(frameI));
    const unsigned expected = (unsigned)std::lround(
      UNIT + ramp.GetGainAt(frameI) * ((int)pLoopRates[offset] - UNIT));

    GOAssert(chunk.pRateUnits[frameI] == expected, "wrong rate in a ramp");
  }
  GOAssert(chunk.nTotalUnits == sum_of(chunk), "wrong sum in a ramp");

  // The stop ramp ends in the middle of the second round: the tail is silent
  run_to_full_depth(fixture);
  fixture.SetEngaged(false);
  fixture.RunRound();
  fixture.RunRound();
  GOAssert(
    fixture.task.GetPitchRateChunk().pRateUnits[N_FRAMES - 1] == UNIT,
    "the tail after the stop ramp must be the unit rate");
}

void GOTestSoundTremulantTaskPitch::TestPitchSharesCursor() {
  GOSoundTremulantTaskTestFixture fixture(TASK_TEST_PITCH_DEVIATION);
  const GOSoundVibratoPitchRateCurve &curve
    = fixture.task.o_WaveForm->GetPitchRateCurve();

  for (unsigned roundN = 1; roundN <= 200; ++roundN) {
    const LoopPosition pos = fixture.task.m_position;

    fixture.SetEngaged((roundN / 7) % 2 == 1);
    fixture.RunRound();

    const ChunkDescription &chunk = fixture.task.GetPitchRateChunk();

    if (chunk.HasRates()) {
      GOAssert(chunk.nTotalUnits == sum_of(chunk), "wrong sum");
      // A chunk at the full depth starts at the cursor of the round
      GOAssert(
        chunk.pRateUnits == fixture.task.o_RampRates.get()
          || chunk.pRateUnits == curve.GetChunk(pos),
        "the chunk must start at the cursor");
    }
  }
}

void GOTestSoundTremulantTaskPitch::TestPitchReversal() {
  GOSoundTremulantTaskTestFixture fixture(TASK_TEST_PITCH_DEVIATION);
  // the deviation is 3 % of the rate
  const unsigned lowSum = N_FRAMES * UNIT * 97 / 100;
  const unsigned highSum = N_FRAMES * UNIT * 103 / 100;

  // Engaged, disengaged in the middle of the start ramp, engaged again
  for (unsigned roundN = 1; roundN <= 4; ++roundN) {
    fixture.SetEngaged(roundN != 2);
    fixture.RunRound();

    const unsigned sum = fixture.task.GetPitchRateChunk().nTotalUnits;

    GOAssert(
      sum >= lowSum && sum <= highSum, "the sum must stay near the unit");
  }
}

void GOTestSoundTremulantTaskPitch::TestPitchChunkEmpty() {
  GOSoundTremulantTaskTestFixture fixture(TASK_TEST_PITCH_DEVIATION);

  // Idle, before the first engagement
  fixture.RunRound();
  GOAssert(
    !fixture.task.GetPitchRateChunk().HasRates(), "an idle task has no pitch");

  // Without pitch
  GOSoundTremulantTaskTestFixture noPitch(0.0f);

  noPitch.SetEngaged(true);
  noPitch.RunRound();
  GOAssert(
    !noPitch.task.GetPitchRateChunk().HasRates(), "no pitch in the form");
  for (unsigned roundI = 0; roundI < 4; ++roundI)
    noPitch.RunRound();
  GOAssert(
    !noPitch.task.GetPitchRateChunk().HasRates(), "no pitch at full depth");

  // A Wave tremulant
  GOTestTremulantModel model;
  const std::unique_ptr<GOTremulant> pWave
    = load_tremulant(model.organModel, "Wave", 100, 4, 4, 20);
  GOSoundTremulantTask waveTask(*pWave, N_FRAMES, TASK_TEST_SAMPLE_RATE);

  GOSoundTremulantTaskTestFixture::RunRound(waveTask);
  GOAssert(!waveTask.GetPitchRateChunk().HasRates(), "a Wave has no pitch");
}

void GOTestSoundTremulantTaskPitch::TestSwapResetsPitchChunk() {
  GOSoundTremulantTaskTestFixture fixture(TASK_TEST_PITCH_DEVIATION);

  fixture.SetEngaged(true);
  fixture.RunRound();
  GOAssert(fixture.task.m_PitchChunk.HasRates(), "the rates were expected");
  fixture.InstallWaveForm(GOSoundTremulantTaskTestFixture::MakeWaveForm(
    N_LOOP,
    TASK_TEST_AMP_DEPTH,
    TASK_TEST_PITCH_DEVIATION,
    TASK_TEST_N_START,
    TASK_TEST_N_STOP));
  GOAssert(
    !fixture.task.m_PitchChunk.HasRates(),
    "a swap must drop the chunk of the old curve");
}

void GOTestSoundTremulantTaskPitch::run() {
  GO_RUN_TEST(TestPitchChunkFullDepth())
  GO_RUN_TEST(TestPitchChunkRamp())
  GO_RUN_TEST(TestPitchSharesCursor())
  GO_RUN_TEST(TestPitchReversal())
  GO_RUN_TEST(TestPitchChunkEmpty())
  GO_RUN_TEST(TestSwapResetsPitchChunk())
}
