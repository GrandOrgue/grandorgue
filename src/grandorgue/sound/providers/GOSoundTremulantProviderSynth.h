/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTPROVIDERSYNTH_H
#define GOSOUNDTREMULANTPROVIDERSYNTH_H

#include "GOSoundTremulantProvider.h"

/**
 * The synthesized tremulant: a sine computed directly at the requested length.
 * The loop holds exactly one period, so it joins itself without resampling.
 * With a = 2 * pi * frameI / nLoopFrames:
 * amp = 1 + ampFactor * sin(a), pitch = pitchCentsFactor * sin(a + phase).
 */
class GOSoundTremulantProviderSynth : public GOSoundTremulantProvider {
public:
  unsigned GetNPeriodsInLoop() const override { return 1; }

  // A sine has no recorded period: Period of the ODF is always required
  float GetNaturalPeriodMs() const override { return 0.0f; }

  void RenderLoop(
    const Scales &scales,
    unsigned nLoopFrames,
    std::vector<float> &outAmp,
    std::vector<float> &outPitchCents) const override;
};

#endif /* GOSOUNDTREMULANTPROVIDERSYNTH_H */
