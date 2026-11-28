/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDCONVERSIONS_H
#define GOTESTSOUNDCONVERSIONS_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundConversions: the dB, cents and milliseconds/frames
 * conversions shared by the model and the sound engine.
 */
class GOTestSoundConversions : public GOTest {
private:
  static const std::string TEST_NAME;

  /** gainToVolume(): 0 dB is exactly 1, +-20 dB is 10 and 0.1, and it is
   * identical to the powf(10, dB / 20) it replaced. gainToVolumeOrMute():
   * the limits and everything outside them. */
  void TestGain();

  /** centsToRatio() and ratioToCents(): the exact octaves and a round trip. */
  void TestCents();

  /** msToFramesRounded() and msToFramesCeil(): exact, fractional and half
   * values, zero, and a long duration at a high sample rate. */
  void TestMsToFrames();

  /** framesToMs() rounds down. */
  void TestFramesToMs();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDCONVERSIONS_H */
