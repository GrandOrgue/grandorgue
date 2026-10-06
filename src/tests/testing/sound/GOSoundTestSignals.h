/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTESTSIGNALS_H
#define GOSOUNDTESTSIGNALS_H

#include <cmath>
#include <vector>

class GOInt24;
class GOSoundBufferPlanarMutable;

/**
 * Generators and measurements of test signals shared by the sound tests. It is
 * a collection of static helpers, not a test: it is not named GOTest... because
 * it is not run by GOTestExe.
 */
class GOSoundTestSignals {
public:
  /**
   * Fills the vector with 0, 1, 2, ...
   * @param outData the vector to fill
   */
  static void fillWithRamp(std::vector<float> &outData);

  /**
   * Fills the vector with the ramp i % 0x7fffff, which fits 24 bits
   * @param outData the vector to fill
   */
  static void fillWithRamp(std::vector<GOInt24> &outData);

  /**
   * Fills all the items of the buffer with +0.5, -0.5, +0.5, ... (the Nyquist
   * signal)
   * @param outBuffer the buffer to fill
   */
  static void fillWithAlternatingSignal(GOSoundBufferPlanarMutable &outBuffer);

  /**
   * @param nFrames the length of the result
   * @param amplitude the amplitude of the sine
   * @param periodFrames the period of the sine, in frames
   * @return nFrames of amplitude * sin(2 * pi * frameI / periodFrames)
   */
  static std::vector<float> makeSinePeriod(
    unsigned nFrames, float amplitude, float periodFrames);

  /**
   * @param cents a pitch shift in cents
   * @return the relative frequency deviation df/f of the shift: 2^(cents/1200)
   *   - 1
   */
  static float centsToDeviation(float cents) {
    return std::exp2(cents / 1200.0f) - 1.0f;
  }
};

#endif /* GOSOUNDTESTSIGNALS_H */
