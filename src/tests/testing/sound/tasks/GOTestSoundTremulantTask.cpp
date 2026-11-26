/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTremulantTask.h"

#include <algorithm>
#include <cmath>

#include "model/GOTremulant.h"
#include "sound/dsp-kernels/GOSoundLinearFaderCore.h"

#include "../GOSoundTestSignals.h"
#include "GOSoundTremulantTaskTestFixture.h"
#include "GOTestScope.h"

const std::string GOTestSoundTremulantTask::TEST_NAME
  = "GOTestSoundTremulantTask";

using LoopPosition = GOSoundTypedBufferLoopBase::LoopPosition;

static constexpr unsigned N_FRAMES = TASK_TEST_N_FRAMES;
static constexpr unsigned N_LOOP = TASK_TEST_N_LOOP;
static constexpr float TOLERANCE = 1e-4f;

static bool are_close(float a, float b) {
  return std::fabs(a - b) <= TOLERANCE;
}

/**
 * @return the amplitude of the loop at the index scaled by the ramp factor
 */
static float amplitude_at(
  const GOSoundTremulantWaveForm &waveForm,
  unsigned loopIndex,
  float rampFactor) {
  LoopPosition pos(waveForm.GetNLoopFrames());

  pos.Advance(loopIndex % waveForm.GetNLoopFrames());
  return waveForm.GetAmplitudeAt(pos, rampFactor);
}

/**
 * @return the advance of the cursor over the first frameI frames of a ramp
 */
static unsigned advance_at(
  const GOSoundLinearFaderCore::UnitRamp &ramp, unsigned frameI) {
  return (unsigned)std::llround(ramp.GetGainSumBefore(frameI));
}

void GOTestSoundTremulantTask::TestWaveTremulant() {
  GOTestTremulantModel model;
  const std::unique_ptr<GOTremulant> pWave
    = load_tremulant(model.organModel, "Wave", 100, 4, 4, 20);
  GOSoundTremulantTask waveTask(*pWave, N_FRAMES, TASK_TEST_SAMPLE_RATE);

  GOAssert(!waveTask.BuildWaveForm(), "a Wave tremulant must have no form");
  GOAssert(!waveTask.o_RampRates, "a Wave tremulant must need no buffer");
  GOSoundTremulantTaskTestFixture::RunRound(waveTask);
  GOAssert(waveTask.GetAmplitude() == 1.0f, "a Wave tremulant gives 1.0");
  GOAssert(
    !waveTask.GetPitchRateChunk().HasRates(), "a Wave tremulant has no pitch");

  GOSoundTremulantTaskTestFixture fixture;

  GOAssert(
    fixture.task.o_RampRates.get(), "a Synth tremulant must have the buffer");
}

void GOTestSoundTremulantTask::TestIdleState() {
  GOSoundTremulantTaskTestFixture fixture;

  for (unsigned roundI = 0; roundI < 5; ++roundI) {
    fixture.RunRound();
    GOAssert(
      fixture.task.GetAmplitude() == 1.0f, "an idle tremulant gives 1.0");
    GOAssert(
      fixture.task.m_position.GetIndex() == 0, "an idle cursor must stay");
  }
}

void GOTestSoundTremulantTask::TestRampUp() {
  GOSoundTremulantTaskTestFixture fixture;
  const GOSoundTremulantWaveForm &waveForm = *fixture.task.o_WaveForm;
  const auto &ramp = waveForm.GetRamp(true);
  unsigned pos = 0;
  unsigned rampFrameI = 0;

  fixture.SetEngaged(true);
  for (unsigned roundI = 0; roundI < 6; ++roundI) {
    float expected;

    if (rampFrameI < TASK_TEST_N_START) {
      const unsigned before = advance_at(ramp, rampFrameI);

      expected = amplitude_at(
        waveForm,
        pos + advance_at(ramp, rampFrameI + N_FRAMES - 1) - before,
        ramp.GetGainAt(rampFrameI + N_FRAMES - 1));
      pos = (pos + advance_at(ramp, rampFrameI + N_FRAMES) - before) % N_LOOP;
      rampFrameI = std::min(rampFrameI + N_FRAMES, TASK_TEST_N_START);
    } else {
      expected = amplitude_at(waveForm, pos + N_FRAMES - 1, 1.0f);
      pos = (pos + N_FRAMES) % N_LOOP;
    }
    fixture.RunRound();
    GOAssert(
      are_close(fixture.task.GetAmplitude(), expected), "wrong amplitude");
    GOAssert(fixture.task.m_position.GetIndex() == pos, "wrong cursor");
    GOAssert(fixture.task.m_RampFrameI == rampFrameI, "wrong ramp position");
  }
  GOAssert(
    fixture.task.m_RampFrameI == TASK_TEST_N_START,
    "the ramp must last exactly the start time");
}

void GOTestSoundTremulantTask::TestRampDown() {
  GOSoundTremulantTaskTestFixture fixture;

  fixture.SetEngaged(true);
  for (unsigned roundI = 0; roundI < 4; ++roundI)
    fixture.RunRound();
  GOAssert(fixture.task.m_position.GetIndex() != 0, "the cursor must move");

  fixture.SetEngaged(false);
  fixture.RunRound();
  GOAssert(
    fixture.task.m_RampFrameI == N_FRAMES, "the stop ramp must be running");
  GOAssert(fixture.task.m_position.GetIndex() != 0, "the ramp must go on");
  // 28 frames of the stop ramp end in the middle of the second round
  fixture.RunRound();
  GOAssert(fixture.task.GetAmplitude() == 1.0f, "the stopped tremulant is 1");
  GOAssert(
    fixture.task.m_position.GetIndex() == 0,
    "the cursor must be reset at the end of the stop ramp");
  fixture.RunRound();
  GOAssert(fixture.task.GetAmplitude() == 1.0f, "the idle tremulant is 1");
  GOAssert(fixture.task.m_position.GetIndex() == 0, "the cursor must stay");
}

void GOTestSoundTremulantTask::TestStartFromAnyPosition() {
  GOSoundTremulantTaskTestFixture fixture;

  fixture.SetEngaged(true);
  for (unsigned roundI = 0; roundI < 5; ++roundI)
    fixture.RunRound();
  fixture.SetEngaged(false);
  fixture.RunRound();

  const unsigned posBefore = fixture.task.m_position.GetIndex();

  GOAssert(posBefore != 0, "the stop ramp must be running");
  fixture.SetEngaged(true);
  fixture.RunRound();

  const unsigned posAfter = fixture.task.m_position.GetIndex();

  GOAssert(posAfter != 0, "the phase must not be reset on engaging");
  GOAssert(
    (posAfter + N_LOOP - posBefore) % N_LOOP <= N_FRAMES,
    "the cursor must go on from the same phase");
}

void GOTestSoundTremulantTask::TestReversalMidRamp() {
  GOSoundTremulantTaskTestFixture fixture;
  const GOSoundTremulantWaveForm &waveForm = *fixture.task.o_WaveForm;
  const auto &startRamp = waveForm.GetRamp(true);
  const auto &stopRamp = waveForm.GetRamp(false);

  fixture.SetEngaged(true);
  fixture.RunRound();
  GOAssert(fixture.task.m_RampFrameI == N_FRAMES, "wrong start ramp position");

  const unsigned mapped
    = GOSoundLinearFaderCore::mapFrameOffset(startRamp, N_FRAMES, stopRamp);

  fixture.SetEngaged(false);
  fixture.RunRound();
  GOAssert(
    fixture.task.m_RampFrameI == std::min(mapped + N_FRAMES, TASK_TEST_N_STOP),
    "the reversal must keep the depth");
  GOAssert(
    std::fabs(stopRamp.GetGainAt(mapped) - startRamp.GetGainAt(N_FRAMES))
      <= 1.0f / TASK_TEST_N_STOP,
    "the depth must be continuous");

  const unsigned mappedBack = GOSoundLinearFaderCore::mapFrameOffset(
    stopRamp, fixture.task.m_RampFrameI, startRamp);

  fixture.SetEngaged(true);
  fixture.RunRound();
  GOAssert(
    fixture.task.m_RampFrameI
      == std::min(mappedBack + N_FRAMES, TASK_TEST_N_START),
    "the second reversal must keep the depth");
}

void GOTestSoundTremulantTask::TestInstantStartStop() {
  GOSoundTremulantTaskTestFixture fixture(0.0f, 0, 0);
  const GOSoundTremulantWaveForm &waveForm = *fixture.task.o_WaveForm;

  fixture.SetEngaged(true);
  fixture.RunRound();
  GOAssert(
    are_close(
      fixture.task.GetAmplitude(), amplitude_at(waveForm, N_FRAMES - 1, 1.0f)),
    "an instant start must give the full depth at once");

  fixture.SetEngaged(false);
  fixture.RunRound();
  GOAssert(fixture.task.GetAmplitude() == 1.0f, "an instant stop must give 1");
  GOAssert(fixture.task.m_position.GetIndex() == 0, "the cursor must reset");
  fixture.RunRound();
  GOAssert(fixture.task.GetAmplitude() == 1.0f, "the idle tremulant is 1");

  // The reversal of a zero-length ramp does not assert
  fixture.SetEngaged(true);
  fixture.RunRound();
  GOAssert(
    are_close(
      fixture.task.GetAmplitude(), amplitude_at(waveForm, N_FRAMES - 1, 1.0f)),
    "an instant restart must give the full depth at once");
}

void GOTestSoundTremulantTask::TestCursorFollowsGainSum() {
  GOSoundTremulantTaskTestFixture fixture;
  const auto &ramp = fixture.task.o_WaveForm->GetRamp(true);
  // The ramp ends in the third round: its last 8 frames run at the full depth
  const unsigned posAfterRamp = advance_at(ramp, 3 * N_FRAMES);

  fixture.SetEngaged(true);
  for (unsigned roundN = 1; roundN <= 200; ++roundN) {
    fixture.RunRound();

    const unsigned expectedPos = roundN < 3
      ? advance_at(ramp, roundN * N_FRAMES) % N_LOOP
      : (posAfterRamp + (roundN - 3) * N_FRAMES) % N_LOOP;

    GOAssert(
      fixture.task.m_position.GetIndex() == expectedPos, "the cursor drifted");
  }
}

void GOTestSoundTremulantTask::TestEnsureDone() {
  GOSoundTremulantTaskTestFixture fixture;

  fixture.SetEngaged(true);
  fixture.task.NewRound();
  GOAssert(fixture.task.m_position.GetIndex() == 0, "the cursor must stay");
  fixture.task.EnsureDone();

  const unsigned posAfterFirst = fixture.task.m_position.GetIndex();
  const float amplitude = fixture.task.GetAmplitude();

  GOAssert(posAfterFirst != 0, "the first EnsureDone() must run the task");
  fixture.task.EnsureDone();
  GOAssert(
    fixture.task.m_position.GetIndex() == posAfterFirst,
    "the second EnsureDone() must not run the task");
  GOAssert(
    fixture.task.GetAmplitude() == amplitude, "the amplitude must not change");
}

void GOTestSoundTremulantTask::TestSwapWaveForm() {
  GOSoundTremulantTaskTestFixture fixture;

  fixture.SetEngaged(true);
  fixture.RunRound();
  fixture.RunRound();

  const unsigned pos = fixture.task.m_position.GetIndex();
  const GOSoundTremulantWaveForm *pOld = fixture.task.o_WaveForm.get();
  auto oLonger = GOSoundTremulantTaskTestFixture::MakeWaveForm(
    2 * N_LOOP, TASK_TEST_AMP_DEPTH, 0.0f, 2 * TASK_TEST_N_START, 0);

  fixture.task.SwapWaveForm(oLonger);
  GOAssert(oLonger.get() == pOld, "the old wave form must come back");
  GOAssert(
    fixture.task.m_position.GetNLoopFrames() == 2 * N_LOOP,
    "wrong new loop length");
  GOAssert(
    fixture.task.m_position.GetIndex() == 2 * pos,
    "the loop fraction must be kept");
  // The depth 32 / 40 = 0.8 is kept on the twice longer ramp
  GOAssert(
    fixture.task.m_RampFrameI == 2 * 2 * N_FRAMES,
    "the depth must be kept on the longer ramp");

  auto oInstant = GOSoundTremulantTaskTestFixture::MakeWaveForm(
    N_LOOP, TASK_TEST_AMP_DEPTH, 0.0f, 0, 0);

  fixture.task.SwapWaveForm(oInstant);
  GOAssert(fixture.task.m_RampFrameI == 0, "a swap onto instant ramps");
  fixture.RunRound();
  GOAssert(fixture.task.GetAmplitude() != 1.0f, "the full depth was expected");

  // From no wave form to a wave form: the task is idle
  GOSoundTremulantTask bareTask(
    fixture.tremulant, N_FRAMES, TASK_TEST_SAMPLE_RATE);
  auto oFirst = GOSoundTremulantTaskTestFixture::MakeWaveForm(
    N_LOOP, TASK_TEST_AMP_DEPTH, 0.0f, TASK_TEST_N_START, TASK_TEST_N_STOP);

  GOAssert(!bareTask.m_position.IsValid(), "a new task has no cursor");
  bareTask.SwapWaveForm(oFirst);
  GOAssert(!oFirst, "a new task has no old wave form");
  GOAssert(bareTask.m_position.IsValid(), "the cursor must be valid");
  GOAssert(bareTask.m_position.GetIndex() == 0, "the cursor must be at 0");
  GOAssert(
    bareTask.m_RampFrameI == TASK_TEST_N_STOP, "the falling ramp must be over");

  // Onto no wave form
  std::unique_ptr<GOSoundTremulantWaveForm> oNone;

  bareTask.SwapWaveForm(oNone);
  GOAssert(oNone.get(), "the wave form must come back");
  GOAssert(!bareTask.m_position.IsValid(), "the cursor must be dropped");
  GOAssert(bareTask.m_RampFrameI == 0, "the ramp position must be dropped");
}

void GOTestSoundTremulantTask::TestSwapOntoInstantStopDuringFallingRamp() {
  GOSoundTremulantTaskTestFixture fixture;
  GOSoundTremulantTaskTestFixture cleanFixture;

  // Reach the full depth, then start the falling ramp and stay inside it
  fixture.SetEngaged(true);
  for (unsigned roundN = 1; roundN <= 4; ++roundN)
    fixture.RunRound();
  fixture.SetEngaged(false);
  fixture.RunRound();
  GOAssert(
    fixture.task.m_position.GetIndex() != 0
      && !fixture.task.o_WaveForm->GetRamp(false).IsOverAt(
        fixture.task.m_RampFrameI),
    "the test must swap in the middle of a falling ramp");

  auto oInstant = GOSoundTremulantTaskTestFixture::MakeWaveForm(
    N_LOOP, TASK_TEST_AMP_DEPTH, 0.0f, TASK_TEST_N_START, 0);
  auto oCleanInstant = GOSoundTremulantTaskTestFixture::MakeWaveForm(
    N_LOOP, TASK_TEST_AMP_DEPTH, 0.0f, TASK_TEST_N_START, 0);

  fixture.task.SwapWaveForm(oInstant);
  cleanFixture.task.SwapWaveForm(oCleanInstant);
  GOAssert(
    fixture.task.m_position.GetIndex() == 0,
    "the cursor must go back to the loop start");

  // The next engagement behaves as the one of a task that has never run
  fixture.SetEngaged(true);
  cleanFixture.SetEngaged(true);
  fixture.RunRound();
  cleanFixture.RunRound();
  GOAssert(
    fixture.task.GetAmplitude() == cleanFixture.task.GetAmplitude(),
    "the engagement must start from the loop start");
}

void GOTestSoundTremulantTask::TestNoNaN() {
  for (unsigned nRampFrames : {4u, 400u}) {
    GOSoundTremulantTaskTestFixture fixture(
      TASK_TEST_PITCH_DEVIATION, nRampFrames, nRampFrames);

    for (unsigned roundN = 1; roundN <= 100; ++roundN) {
      fixture.SetEngaged((roundN / 25) % 2 == 0);
      fixture.RunRound();

      const float amplitude = fixture.task.GetAmplitude();
      const auto &chunk = fixture.task.GetPitchRateChunk();

      GOAssert(
        std::isfinite(amplitude) && amplitude > 1.0f - 2 * TASK_TEST_AMP_DEPTH
          && amplitude < 1.0f + 2 * TASK_TEST_AMP_DEPTH,
        "wrong amplitude");
      GOAssert(
        !chunk.HasRates() || chunk.nTotalUnits > 0, "wrong pitch rate sum");
    }
  }
}

void GOTestSoundTremulantTask::TestBuildWaveForm() {
  GOTestTremulantModel model;
  const std::unique_ptr<GOTremulant> pSynth
    = load_tremulant(model.organModel, "Synth", 100, 4, 4, 20);
  GOSoundTremulantTask task(*pSynth, N_FRAMES, TASK_TEST_SAMPLE_RATE);
  const auto oWaveForm = task.BuildWaveForm();
  const GOTremulant::Description desc = pSynth->Render(TASK_TEST_SAMPLE_RATE);

  GOAssert(oWaveForm.get(), "a Synth tremulant must have a wave form");
  GOAssert(
    oWaveForm->GetNLoopFrames() == desc.nPeriodFrames, "wrong loop length");
  GOAssert(oWaveForm->GetNLoopFrames() == 4800, "100 ms are 4800 frames");
}

void GOTestSoundTremulantTask::TestStableBuffers() {
  GOSoundTremulantTaskTestFixture fixture(TASK_TEST_PITCH_DEVIATION);
  const uint16_t *pRampRates = fixture.task.o_RampRates.get();

  fixture.SetEngaged(true);
  fixture.RunRound();
  GOAssert(
    fixture.task.m_PitchChunk.pRateUnits == pRampRates,
    "a ramp must use the task buffer");
  for (unsigned roundI = 0; roundI < 3; ++roundI)
    fixture.RunRound();

  const GOSoundTremulantWaveForm &waveForm = *fixture.task.o_WaveForm;
  const LoopPosition pos = fixture.task.m_position;

  fixture.RunRound();
  GOAssert(
    fixture.task.m_PitchChunk.pRateUnits
      == waveForm.GetPitchRateCurve().GetChunk(pos),
    "the full depth must point into the curve");
  fixture.InstallWaveForm(GOSoundTremulantTaskTestFixture::MakeWaveForm(
    N_LOOP, TASK_TEST_AMP_DEPTH, TASK_TEST_PITCH_DEVIATION, 10, 10));
  GOAssert(
    fixture.task.o_RampRates.get() == pRampRates,
    "the buffer must not be reallocated");
}

void GOTestSoundTremulantTask::TestNoAmplitude() {
  // The form modulates only the pitch: the gain stays 1.0 through the ramps
  // and at the full engagement
  GOTremulant::Description desc;

  desc.nPeriodFrames = N_LOOP;
  desc.pitchDeviation = GOSoundTestSignals::makeSinePeriod(
    N_LOOP, TASK_TEST_PITCH_DEVIATION, N_LOOP);
  desc.nStartFrames = TASK_TEST_N_START;
  desc.nStopFrames = TASK_TEST_N_STOP;

  GOSoundTremulantTaskTestFixture fixture;

  fixture.InstallWaveForm(
    std::make_unique<GOSoundTremulantWaveForm>(desc, N_FRAMES));
  fixture.SetEngaged(true);
  for (unsigned roundI = 0; roundI < 6; ++roundI) {
    fixture.RunRound();
    GOAssert(
      fixture.task.GetAmplitude() == 1.0f,
      "no amplitude must give the gain 1.0 at any ramp depth");
  }
  GOAssert(
    fixture.task.GetPitchRateChunk().HasRates(), "the pitch must still work");
}

void GOTestSoundTremulantTask::run() {
  GO_RUN_TEST(TestWaveTremulant())
  GO_RUN_TEST(TestIdleState())
  GO_RUN_TEST(TestRampUp())
  GO_RUN_TEST(TestRampDown())
  GO_RUN_TEST(TestStartFromAnyPosition())
  GO_RUN_TEST(TestReversalMidRamp())
  GO_RUN_TEST(TestInstantStartStop())
  GO_RUN_TEST(TestCursorFollowsGainSum())
  GO_RUN_TEST(TestEnsureDone())
  GO_RUN_TEST(TestSwapWaveForm())
  GO_RUN_TEST(TestSwapOntoInstantStopDuringFallingRamp())
  GO_RUN_TEST(TestNoNaN())
  GO_RUN_TEST(TestBuildWaveForm())
  GO_RUN_TEST(TestStableBuffers())
  GO_RUN_TEST(TestNoAmplitude())
}
