/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTremulantWaveForm.h"

#include <cmath>
#include <vector>

#include "model/GOTremulant.h"
#include "sound/buffer/GOSoundTypedBufferLoopBase.h"
#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/tasks/GOSoundTremulantWaveForm.h"

#include "../GOSoundTestSignals.h"
#include "GOTestScope.h"

const std::string GOTestSoundTremulantWaveForm::TEST_NAME
  = "GOTestSoundTremulantWaveForm";

using LoopPosition = GOSoundTypedBufferLoopBase::LoopPosition;

static constexpr unsigned N_CHUNK = 4;
static constexpr float TOLERANCE = 1e-4f;
static constexpr unsigned UNIT = GOSoundTremulantWaveForm::UNIT_AMP;

static bool are_close(float a, float b) {
  return std::fabs(a - b) <= TOLERANCE;
}

/**
 * Makes a description by hand
 *
 * @param nPeriodFrames the length of the period
 * @param ampDeviation the relative amplitude deviation; empty means no AM
 * @param pitchDeviation the relative frequency deviation; empty means no PM
 * @param nStartFrames the start ramp length
 * @param nStopFrames the stop ramp length
 */
static GOTremulant::Description make_desc(
  unsigned nPeriodFrames,
  const std::vector<float> &ampDeviation,
  const std::vector<float> &pitchDeviation,
  unsigned nStartFrames,
  unsigned nStopFrames) {
  GOTremulant::Description desc;

  desc.nPeriodFrames = nPeriodFrames;
  desc.ampDeviation = ampDeviation;
  desc.pitchDeviation = pitchDeviation;
  desc.nStartFrames = nStartFrames;
  desc.nStopFrames = nStopFrames;
  return desc;
}

/**
 * @return the unit value of the loop at the position
 */
static unsigned unit_at(
  const GOSoundTremulantWaveForm &waveForm, const LoopPosition &pos) {
  return *waveForm.GetAmpUnits().GetChunk(pos);
}

void GOTestSoundTremulantWaveForm::TestLoopLength() {
  for (unsigned nLoopFrames : {1u, 7u, 8u, 1001u}) {
    const GOSoundTremulantWaveForm waveForm(
      make_desc(nLoopFrames, std::vector<float>(nLoopFrames, 0.0f), {}, 0, 0),
      N_CHUNK);

    GOAssert(waveForm.GetNLoopFrames() == nLoopFrames, "wrong loop length");
    GOAssert(
      waveForm.GetAmpUnits().GetNChunkFrames() == N_CHUNK,
      "wrong chunk length");
    GOAssert(waveForm.GetAmpUnits().IsValid(), "the loop must be valid");
  }
}

void GOTestSoundTremulantWaveForm::TestAmplitudeConversion() {
  const std::vector<float> deviations{0.0f, -0.5f, 0.25f, -1.0f};

  for (float deviation : deviations) {
    const GOSoundTremulantWaveForm waveForm(
      make_desc(8, std::vector<float>(8, deviation), {}, 0, 0), N_CHUNK);
    LoopPosition pos(8);

    for (unsigned frameI = 0; frameI < 8; ++frameI, pos.Advance(1))
      GOAssert(
        unit_at(waveForm, pos)
          == (unsigned)std::lround((1.0f + deviation) * UNIT),
        "wrong amplitude units");
  }
}

void GOTestSoundTremulantWaveForm::TestAmplitudeClamping() {
  const GOSoundTremulantWaveForm waveForm(
    make_desc(4, {-1.5f, 9.0f, 0.0f, 0.0f}, {}, 0, 0), N_CHUNK);
  LoopPosition pos(4);

  GOAssert(unit_at(waveForm, pos) == 0, "a negative amplitude must clamp to 0");
  pos.Advance(1);
  GOAssert(
    unit_at(waveForm, pos) == UINT16_MAX,
    "a too large amplitude must clamp to UINT16_MAX");
}

void GOTestSoundTremulantWaveForm::TestAmplitudeTail() {
  const std::vector<float> deviations{-0.9f, -0.8f, -0.7f, -0.6f, -0.5f, -0.4f};

  const GOSoundTremulantWaveForm waveForm(
    make_desc(6, deviations, {}, 0, 0), N_CHUNK);
  const GOSoundTypedBufferLoopMonoManaged<uint16_t> &ampUnits
    = waveForm.GetAmpUnits();
  LoopPosition pos(6);

  // The chunk from the last frames continues with the loop start
  pos.Advance(5);

  const uint16_t *pChunk = ampUnits.GetChunk(pos);

  GOAssert(
    pChunk[0] == std::lround((1.0f + deviations[5]) * UNIT),
    "wrong chunk start");
  GOAssert(
    pChunk[1] == std::lround((1.0f + deviations[0]) * UNIT),
    "the tail must wrap");
  GOAssert(
    pChunk[2] == std::lround((1.0f + deviations[1]) * UNIT),
    "the tail must wrap");
  GOAssert(
    pChunk[3] == std::lround((1.0f + deviations[2]) * UNIT),
    "the tail must wrap");

  // A loop shorter than the chunk repeats cyclically
  const std::vector<float> shortDeviations{-0.75f, -0.25f};

  const GOSoundTremulantWaveForm shortForm(
    make_desc(2, shortDeviations, {}, 0, 0), N_CHUNK);
  LoopPosition shortPos(2);
  const uint16_t *pShort = shortForm.GetAmpUnits().GetChunk(shortPos);

  for (unsigned frameI = 0; frameI < N_CHUNK; ++frameI)
    GOAssert(
      pShort[frameI]
        == std::lround((1.0f + shortDeviations[frameI % 2]) * UNIT),
      "a short loop must repeat cyclically");
}

void GOTestSoundTremulantWaveForm::TestRamps() {
  const GOSoundTremulantWaveForm waveForm(
    make_desc(8, std::vector<float>(8, 0.0f), {}, 40, 28), N_CHUNK);
  const auto &startRamp = waveForm.GetRamp(true);
  const auto &stopRamp = waveForm.GetRamp(false);

  GOAssert(startRamp.IsRising(), "the start ramp must rise");
  GOAssert(startRamp.GetNRampFrames() == 40, "wrong start ramp length");
  GOAssert(startRamp.GetGainAt(0) == 0.0f, "the start ramp must begin at 0");
  GOAssert(startRamp.GetGainAt(40) == 1.0f, "the start ramp must end at 1");
  GOAssert(!stopRamp.IsRising(), "the stop ramp must fall");
  GOAssert(stopRamp.GetNRampFrames() == 28, "wrong stop ramp length");
  GOAssert(stopRamp.GetGainAt(0) == 1.0f, "the stop ramp must begin at 1");
  GOAssert(stopRamp.GetGainAt(28) == 0.0f, "the stop ramp must end at 0");
}

void GOTestSoundTremulantWaveForm::TestZeroRamps() {
  const GOSoundTremulantWaveForm waveForm(
    make_desc(8, std::vector<float>(8, 0.0f), {}, 0, 0), N_CHUNK);
  const auto &startRamp = waveForm.GetRamp(true);
  const auto &stopRamp = waveForm.GetRamp(false);

  GOAssert(startRamp.GetNRampFrames() == 0, "the start ramp must be instant");
  GOAssert(startRamp.GetGainAt(0) == 1.0f, "an instant start ramp is 1");
  GOAssert(startRamp.IsRising(), "an instant start ramp must keep its sense");
  GOAssert(stopRamp.GetNRampFrames() == 0, "the stop ramp must be instant");
  GOAssert(stopRamp.GetGainAt(0) == 0.0f, "an instant stop ramp is 0");
  GOAssert(!stopRamp.IsRising(), "an instant stop ramp must keep its sense");
}

void GOTestSoundTremulantWaveForm::TestGetAmplitudeAtDepth() {
  for (float deviation : {0.2f, -0.2f}) {
    const GOSoundTremulantWaveForm waveForm(
      make_desc(8, std::vector<float>(8, deviation), {}, 0, 0), N_CHUNK);
    const LoopPosition pos(8);
    // the value after the conversion to units
    const float loopValue
      = (float)std::lround((1.0f + deviation) * UNIT) / UNIT;

    GOAssert(
      are_close(waveForm.GetAmplitudeAt(pos, 1.0f), loopValue),
      "depth 1 must give the loop value");
    GOAssert(
      waveForm.GetAmplitudeAt(pos, 0.0f) == 1.0f, "depth 0 must give 1.0");
    GOAssert(
      are_close(
        waveForm.GetAmplitudeAt(pos, 0.5f), 1.0f + 0.5f * (loopValue - 1.0f)),
      "depth 0.5 must give the middle");
  }
}

void GOTestSoundTremulantWaveForm::TestGetAmplitudeAtPositions() {
  // The deviation profile: the gain is 1 + deviation
  const std::vector<float> profile{
    0.0f, 0.1f, 0.2f, 0.1f, 0.0f, -0.1f, -0.2f, -0.1f};
  const GOSoundTremulantWaveForm waveForm(
    make_desc(8, profile, {}, 0, 0), N_CHUNK);
  LoopPosition pos(8);

  for (unsigned frameI = 0; frameI < 8; ++frameI, pos.Advance(1))
    GOAssert(
      are_close(waveForm.GetAmplitudeAt(pos, 1.0f), 1.0f + profile[frameI]),
      "wrong amplitude at a position");
}

void GOTestSoundTremulantWaveForm::TestPitch() {
  constexpr unsigned N_LOOP = 64;
  const std::vector<float> pitchDeviation = GOSoundTestSignals::makeSinePeriod(
    N_LOOP, GOSoundTestSignals::centsToDeviation(50.0f), N_LOOP);

  const GOSoundTremulantWaveForm waveForm(
    make_desc(N_LOOP, std::vector<float>(N_LOOP, 0.0f), pitchDeviation, 0, 0),
    N_CHUNK);
  const GOSoundVibratoPitchRateCurve &curve = waveForm.GetPitchRateCurve();
  LoopPosition pos(N_LOOP);
  unsigned long sumUnits = 0;

  GOAssert(waveForm.HasPitch(), "the form must have pitch");
  GOAssert(curve.GetNLoopFrames() == N_LOOP, "wrong curve length");
  GOAssert(curve.GetNChunkFrames() == N_CHUNK, "wrong curve chunk length");
  for (unsigned frameI = 0; frameI < N_LOOP; ++frameI, pos.Advance(1)) {
    // one position reads both loops
    GOAssert(waveForm.GetAmplitudeAt(pos, 1.0f) == 1.0f, "wrong amplitude");
    sumUnits += *curve.GetChunk(pos);
  }
  GOAssert(
    sumUnits == (unsigned long)N_LOOP * GOSoundResample::UPSAMPLE_FACTOR,
    "the curve must be normalised");
}

void GOTestSoundTremulantWaveForm::TestNoPitch() {
  const GOSoundTremulantWaveForm waveForm(
    make_desc(8, std::vector<float>(8, 0.1f), {}, 0, 0), N_CHUNK);

  GOAssert(!waveForm.HasPitch(), "the form must have no pitch");
  GOAssert(
    !waveForm.GetPitchRateCurve().IsValid(), "the curve must not be built");
  GOAssert(waveForm.GetAmpUnits().IsValid(), "the amplitude must be built");
}

void GOTestSoundTremulantWaveForm::TestNoAmplitude() {
  constexpr unsigned N_PERIOD = 8;

  const GOSoundTremulantWaveForm waveForm(
    make_desc(
      N_PERIOD, {}, {1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f}, 0, 0),
    N_CHUNK);
  const LoopPosition pos(N_PERIOD);

  GOAssert(!waveForm.HasAmplitude(), "the form must have no amplitude");
  GOAssert(!waveForm.GetAmpUnits().IsValid(), "the loop must not be built");
  GOAssert(waveForm.GetNLoopFrames() == N_PERIOD, "wrong loop length");
  GOAssert(waveForm.HasPitch(), "the pitch must be built");
  GOAssert(
    waveForm.GetAmplitudeAt(pos, 1.0f) == 1.0f,
    "no amplitude must give the gain 1.0 at any depth");
}

void GOTestSoundTremulantWaveForm::run() {
  GO_RUN_TEST(TestLoopLength())
  GO_RUN_TEST(TestAmplitudeConversion())
  GO_RUN_TEST(TestAmplitudeClamping())
  GO_RUN_TEST(TestAmplitudeTail())
  GO_RUN_TEST(TestRamps())
  GO_RUN_TEST(TestZeroRamps())
  GO_RUN_TEST(TestGetAmplitudeAtDepth())
  GO_RUN_TEST(TestGetAmplitudeAtPositions())
  GO_RUN_TEST(TestPitch())
  GO_RUN_TEST(TestNoPitch())
  GO_RUN_TEST(TestNoAmplitude())
}
