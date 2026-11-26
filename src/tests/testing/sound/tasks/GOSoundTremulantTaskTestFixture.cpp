/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantTaskTestFixture.h"

#include <cmath>
#include <numbers>

/**
 * Makes a description by hand (no GOTremulant::Render()): the amplitude
 * deviation is ampDepth * sin and the pitch deviation is pitchDeviation * sin
 * over one period of the loop.
 */
static GOTremulant::Description make_description(
  unsigned nLoopFrames,
  float ampDepth,
  float pitchDeviation,
  unsigned nStartFrames,
  unsigned nStopFrames) {
  GOTremulant::Description desc;
  const double radsPerFrame = 2.0 * std::numbers::pi / nLoopFrames;

  desc.nPeriodFrames = nLoopFrames;
  desc.ampDeviation.resize(nLoopFrames);
  for (unsigned frameI = 0; frameI < nLoopFrames; ++frameI)
    desc.ampDeviation[frameI]
      = ampDepth * (float)std::sin(radsPerFrame * frameI);
  if (pitchDeviation != 0.0f) {
    desc.pitchDeviation.resize(nLoopFrames);
    for (unsigned frameI = 0; frameI < nLoopFrames; ++frameI)
      desc.pitchDeviation[frameI]
        = pitchDeviation * (float)std::sin(radsPerFrame * frameI);
  }
  desc.nStartFrames = nStartFrames;
  desc.nStopFrames = nStopFrames;
  return desc;
}

std::unique_ptr<GOSoundTremulantWaveForm> GOSoundTremulantTaskTestFixture::
  MakeWaveForm(
    unsigned nLoopFrames,
    float ampDepth,
    float pitchDeviation,
    unsigned nStartFrames,
    unsigned nStopFrames) {
  return std::make_unique<GOSoundTremulantWaveForm>(
    make_description(
      nLoopFrames, ampDepth, pitchDeviation, nStartFrames, nStopFrames),
    TASK_TEST_N_FRAMES);
}

void GOSoundTremulantTaskTestFixture::RunRound(GOSoundTremulantTask &task) {
  task.NewRound();
  task.EnsureDone();
}

GOSoundTremulantTaskTestFixture::GOSoundTremulantTaskTestFixture(
  float pitchDeviation,
  unsigned nStartFrames,
  unsigned nStopFrames,
  unsigned nLoopFrames)
  : tremulant(model.organModel),
    task(tremulant, TASK_TEST_N_FRAMES, TASK_TEST_SAMPLE_RATE) {
  InstallWaveForm(MakeWaveForm(
    nLoopFrames,
    TASK_TEST_AMP_DEPTH,
    pitchDeviation,
    nStartFrames,
    nStopFrames));
}

void GOSoundTremulantTaskTestFixture::InstallWaveForm(
  std::unique_ptr<GOSoundTremulantWaveForm> oWaveForm) {
  task.SwapWaveForm(oWaveForm);
}
