/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTREMULANTWAVEFORM_H
#define GOTESTSOUNDTREMULANTWAVEFORM_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundTremulantWaveForm built from hand-made descriptions: the
 * conversion of the amplitude deviation to units, the tail of the loop, the
 * depth ramps and the pitch curve.
 */
class GOTestSoundTremulantWaveForm : public GOTest {
private:
  static const std::string TEST_NAME;

  /** The loop and the chunk lengths are those of the description. */
  void TestLoopLength();

  /** A relative amplitude deviation becomes UNIT_AMP * (1 + deviation) units.
   */
  void TestAmplitudeConversion();

  /** Values outside the range of uint16_t are clamped. */
  void TestAmplitudeClamping();

  /** A chunk across the seam continues with the loop start. */
  void TestAmplitudeTail();

  /** The start ramp rises and the stop ramp falls over their lengths. */
  void TestRamps();

  /** Zero-length ramps are constant and keep their direction. */
  void TestZeroRamps();

  /** GetAmplitudeAt() scales the deviation from 1.0 by the ramp factor. */
  void TestGetAmplitudeAtDepth();

  /** GetAmplitudeAt() reads a profile position by position. */
  void TestGetAmplitudeAtPositions();

  /** A description with pitch builds the normalised rate curve. */
  void TestPitch();

  /** A description without pitch leaves the curve not built. */
  void TestNoPitch();

  /** A description without amplitude leaves the loop not built. */
  void TestNoAmplitude();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTREMULANTWAVEFORM_H */
