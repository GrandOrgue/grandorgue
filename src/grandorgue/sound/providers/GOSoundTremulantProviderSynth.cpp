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

GOSoundTremulantProvider::Shapes GOSoundTremulantProviderSynth::RenderOnePeriod(
  unsigned nPeriodFrames, bool isAmpNeeded, bool isPitchNeeded) const {
  assert(nPeriodFrames > 0);

  // The phase change per frame, in radians: one period per call
  const double radsPerFrame = 2.0 * std::numbers::pi / nPeriodFrames;
  Shapes shapes;

  if (isAmpNeeded)
    shapes.ampDeviation.resize(nPeriodFrames);
  if (isPitchNeeded)
    shapes.pitchDeviation.resize(nPeriodFrames);

  float *pAmp = isAmpNeeded ? shapes.ampDeviation.data() : nullptr;
  float *pPitch = isPitchNeeded ? shapes.pitchDeviation.data() : nullptr;

  for (unsigned frameI = 0; frameI < nPeriodFrames; ++frameI) {
    const float value = (float)std::sin(radsPerFrame * frameI);

    if (pAmp)
      *pAmp++ = value;
    if (pPitch)
      *pPitch++ = value;
  }
  return shapes;
}
