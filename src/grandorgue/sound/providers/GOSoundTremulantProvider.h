/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTPROVIDER_H
#define GOSOUNDTREMULANTPROVIDER_H

#include <vector>

/**
 * A source of one loop of a tremulant wave form: the amplitude multiplier and
 * the pitch deviation at the same frame indexes. Owned by GOTremulant and
 * called only from the control thread while a wave form is being built; the
 * audio thread never sees a provider, it reads the rendered loop.
 * Implementations hold no mutable state, so RenderLoop() may run concurrently.
 */
class GOSoundTremulantProvider {
public:
  /**
   * The numbers that scale the unit-depth shape of a loop. Filled by
   * GOTremulant::Render() from the tremulant parameters, which converts the
   * user-visible units (percent, degrees, the scale of an LFO file) to these.
   */
  struct Scales {
    // amplitude depth as a fraction: the multiplier varies as 1 +- this
    float ampFactor;
    // peak pitch deviation, in cents; 0 means no pitch modulation
    float pitchCentsFactor;
    // pitch phase against the amplitude; positive: the pitch leads
    float pitchPhaseRads;
  };

  virtual ~GOSoundTremulantProvider() = default;

  /** @return the number of modulation periods in one loop, >= 1 */
  virtual unsigned GetNPeriodsInLoop() const = 0;

  /**
   * The period of the modulation as recorded in the source of the shape, e.g.
   * 1000 / f_file for an LFO wav file. GOTremulant::Render() uses it as the
   * period when the ODF does not give one, so that the tremulant sounds as it
   * was recorded; a given period stretches the shape instead.
   *
   * @return the natural period in ms, 0 if the shape has none, so the period
   *   must be given by the user
   */
  virtual float GetNaturalPeriodMs() const = 0;

  /**
   * Renders one loop.
   *
   * @param scales the depths and the phase
   * @param nLoopFrames the loop length in frames, > 0
   * @param outAmp the amplitude multiplier per frame, 1.0 = no modulation;
   *   resized to nLoopFrames
   * @param outPitchCents the pitch deviation per frame, in cents; resized to
   *   nLoopFrames, or emptied if scales.pitchCentsFactor is 0
   */
  virtual void RenderLoop(
    const Scales &scales,
    unsigned nLoopFrames,
    std::vector<float> &outAmp,
    std::vector<float> &outPitchCents) const = 0;
};

#endif /* GOSOUNDTREMULANTPROVIDER_H */
