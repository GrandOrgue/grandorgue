/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantProviderSynth.h"

#include <cassert>
#include <cmath>
#include <numbers>

void GOSoundTremulantProviderSynth::RenderLoop(
  const Scales &scales,
  unsigned nLoopFrames,
  std::vector<float> &outAmp,
  std::vector<float> &outPitchCents) const {
  assert(nLoopFrames > 0);

  // The phase change per frame, in radians: one period per loop
  const double radsPerFrame = 2.0 * std::numbers::pi / nLoopFrames;

  // Computing the amplitude part
  outAmp.resize(nLoopFrames);

  float *pAmp = outAmp.data();

  for (unsigned frameI = 0; frameI < nLoopFrames; ++frameI, ++pAmp)
    *pAmp = 1.0f + scales.ampFactor * (float)std::sin(radsPerFrame * frameI);

  // Computing the pitch part
  if (scales.pitchCentsFactor != 0.0f) {
    outPitchCents.resize(nLoopFrames);

    float *pPitch = outPitchCents.data();

    for (unsigned frameI = 0; frameI < nLoopFrames; ++frameI, ++pPitch)
      *pPitch = scales.pitchCentsFactor
        * (float)std::sin(radsPerFrame * frameI + scales.pitchPhaseRads);
  } else
    outPitchCents.clear();
}
