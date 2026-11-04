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
 * A source of one period of the tremulant deviations: the amplitude deviation
 * and the pitch deviation at the same frame indexes, in the provider's
 * natural scale. The caller multiplies a deviation by a depth to get the
 * relative deviation around zero; the nominal level and frequency are zero
 * deviation.
 *
 * Owned by GOTremulant and called only from the control thread while a wave
 * form is being built; the audio thread never sees a provider, it reads the
 * rendered loop. Implementations hold no mutable state, so RenderOnePeriod()
 * may run concurrently.
 */
class GOSoundTremulantProvider {
public:
  /**
   * One period of the deviations. A deviation that was not requested or that
   * the source does not have is empty.
   */
  struct Shapes {
    std::vector<float> ampDeviation;
    std::vector<float> pitchDeviation;
  };

  virtual ~GOSoundTremulantProvider() = default;

  /**
   * Renders exactly one period of the deviations, in their natural phase
   * relation (the user's phase shift is applied by the caller).
   *
   * @param nPeriodFrames the period length in frames, > 0; the caller rounds
   *   the period to whole frames
   * @param isAmpNeeded whether the amplitude deviation is to be rendered
   * @param isPitchNeeded whether the pitch deviation is to be rendered
   * @return the deviations in the provider's natural scale, each empty or
   *   nPeriodFrames long; a deviation that is not needed is not computed and
   *   is empty
   */
  virtual Shapes RenderOnePeriod(
    unsigned nPeriodFrames, bool isAmpNeeded, bool isPitchNeeded) const = 0;
};

#endif /* GOSOUNDTREMULANTPROVIDER_H */
