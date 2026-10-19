/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTremulantProviderSynth.h"

#include <cmath>
#include <numbers>
#include <numeric>
#include <vector>

#include "sound/providers/GOSoundTremulantProviderSynth.h"

#include "GOTestScope.h"

const std::string GOTestSoundTremulantProviderSynth::TEST_NAME
  = "GOTestSoundTremulantProviderSynth";

using Scales = GOSoundTremulantProvider::Scales;

static constexpr unsigned N_LOOP = 64;
static constexpr float TOLERANCE = 1e-5f;

static bool are_close(float a, float b) {
  return std::fabs(a - b) <= TOLERANCE * std::max(1.0f, std::fabs(b));
}

/**
 * @return the sine of the frame of a loop with one period
 */
static float sine_at(unsigned frameI, unsigned nLoopFrames, float phaseRads) {
  return (float)std::sin(
    2.0 * std::numbers::pi * frameI / nLoopFrames + phaseRads);
}

void GOTestSoundTremulantProviderSynth::TestLoopDescription() {
  const GOSoundTremulantProviderSynth synth;
  const GOSoundTremulantProvider &provider = synth;

  GOAssert(provider.GetNPeriodsInLoop() == 1, "a loop must hold one period");
  GOAssert(
    provider.GetNaturalPeriodMs() == 0.0f,
    "a sine must have no natural period");
}

void GOTestSoundTremulantProviderSynth::TestLoopLength() {
  const GOSoundTremulantProviderSynth synth;
  const GOSoundTremulantProvider &provider = synth;
  const Scales scales{0.2f, 25.0f, 0.0f};

  for (unsigned nLoopFrames : {1u, 2u, 7u, 64u, 1001u}) {
    std::vector<float> amp;
    std::vector<float> pitch;

    provider.RenderLoop(scales, nLoopFrames, amp, pitch);
    GOAssert(amp.size() == nLoopFrames, "wrong amplitude length");
    GOAssert(pitch.size() == nLoopFrames, "wrong pitch length");
  }
}

void GOTestSoundTremulantProviderSynth::TestAmplitudeShape() {
  const GOSoundTremulantProviderSynth synth;
  const GOSoundTremulantProvider &provider = synth;
  const float depth = 0.2f;
  const Scales scales{depth, 0.0f, 0.0f};
  std::vector<float> amp;
  std::vector<float> pitch;

  provider.RenderLoop(scales, N_LOOP, amp, pitch);
  GOAssert(amp[0] == 1.0f, "the loop must start at 1");
  GOAssert(are_close(amp[N_LOOP / 4], 1.0f + depth), "wrong maximum");
  GOAssert(are_close(amp[3 * N_LOOP / 4], 1.0f - depth), "wrong minimum");

  const double sum = std::accumulate(amp.begin(), amp.end(), 0.0);

  GOAssert(std::fabs(sum / N_LOOP - 1.0) < 1e-5, "the mean must be 1");
  for (unsigned frameI = 0; frameI < N_LOOP; ++frameI)
    GOAssert(
      are_close(amp[frameI], 1.0f + depth * sine_at(frameI, N_LOOP, 0.0f)),
      "the amplitude must follow 1 + depth * sin");
}

void GOTestSoundTremulantProviderSynth::TestZeroAmplitudeDepth() {
  const GOSoundTremulantProviderSynth synth;
  const GOSoundTremulantProvider &provider = synth;
  const Scales scales{0.0f, 0.0f, 0.0f};
  std::vector<float> amp;
  std::vector<float> pitch;

  provider.RenderLoop(scales, N_LOOP, amp, pitch);
  for (float value : amp)
    GOAssert(value == 1.0f, "a zero depth must give a flat 1.0");
}

void GOTestSoundTremulantProviderSynth::TestPitchPresence() {
  const GOSoundTremulantProviderSynth synth;
  const GOSoundTremulantProvider &provider = synth;
  std::vector<float> amp;
  std::vector<float> pitch;

  provider.RenderLoop(Scales{0.2f, 0.0f, 0.0f}, N_LOOP, amp, pitch);
  GOAssert(pitch.empty(), "0 cents must give no pitch");

  provider.RenderLoop(Scales{0.2f, 25.0f, 0.0f}, N_LOOP, amp, pitch);
  GOAssert(pitch.size() == N_LOOP, "the pitch must have the loop length");
  GOAssert(are_close(pitch[N_LOOP / 4], 25.0f), "wrong positive peak");
  GOAssert(are_close(pitch[3 * N_LOOP / 4], -25.0f), "wrong negative peak");
}

void GOTestSoundTremulantProviderSynth::TestPitchPhase() {
  const GOSoundTremulantProviderSynth synth;
  const GOSoundTremulantProvider &provider = synth;
  const float cents = 25.0f;
  const float halfPi = (float)(std::numbers::pi / 2.0);
  const float pi = (float)std::numbers::pi;
  std::vector<float> amp;
  std::vector<float> pitch0;
  std::vector<float> pitchHalfPi;
  std::vector<float> pitchPi;

  provider.RenderLoop(Scales{0.2f, cents, 0.0f}, N_LOOP, amp, pitch0);
  provider.RenderLoop(Scales{0.2f, cents, halfPi}, N_LOOP, amp, pitchHalfPi);
  provider.RenderLoop(Scales{0.2f, cents, pi}, N_LOOP, amp, pitchPi);

  GOAssert(pitch0[0] == 0.0f, "phase 0 must start at 0 cents");
  GOAssert(are_close(pitchHalfPi[0], cents), "pi/2 must lead to the peak");
  GOAssert(
    std::fabs(pitchHalfPi[N_LOOP / 4]) < 1e-3f,
    "pi/2 must be zero where the amplitude peaks");
  for (unsigned frameI = 0; frameI < N_LOOP; ++frameI)
    GOAssert(
      std::fabs(pitchPi[frameI] + pitch0[frameI]) < 1e-3f,
      "phase pi must be the negative of phase 0");
}

void GOTestSoundTremulantProviderSynth::TestOutputReuse() {
  const GOSoundTremulantProviderSynth synth;
  const GOSoundTremulantProvider &provider = synth;
  std::vector<float> amp(5000, 7.0f);
  std::vector<float> pitch(5000, 7.0f);

  provider.RenderLoop(Scales{0.2f, 25.0f, 0.0f}, N_LOOP, amp, pitch);
  GOAssert(amp.size() == N_LOOP, "old longer amplitude must be cut");
  GOAssert(pitch.size() == N_LOOP, "old longer pitch must be cut");
  GOAssert(amp[0] == 1.0f, "old amplitude values must be overwritten");
  GOAssert(pitch[0] == 0.0f, "old pitch values must be overwritten");

  provider.RenderLoop(Scales{0.2f, 0.0f, 0.0f}, N_LOOP, amp, pitch);
  GOAssert(pitch.empty(), "a stale pitch must be emptied at 0 cents");
}

void GOTestSoundTremulantProviderSynth::run() {
  GO_RUN_TEST(TestLoopDescription())
  GO_RUN_TEST(TestLoopLength())
  GO_RUN_TEST(TestAmplitudeShape())
  GO_RUN_TEST(TestZeroAmplitudeDepth())
  GO_RUN_TEST(TestPitchPresence())
  GO_RUN_TEST(TestPitchPhase())
  GO_RUN_TEST(TestOutputReuse())
}
