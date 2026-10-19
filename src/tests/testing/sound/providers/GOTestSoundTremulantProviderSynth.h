/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTREMULANTPROVIDERSYNTH_H
#define GOTESTSOUNDTREMULANTPROVIDERSYNTH_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundTremulantProviderSynth through the interface of
 * GOSoundTremulantProvider: the description, the loop length, the amplitude
 * and the pitch shapes, the pitch phase and the reuse of the output vectors.
 */
class GOTestSoundTremulantProviderSynth : public GOTest {
private:
  static const std::string TEST_NAME;

  /** One period per loop, no natural period. */
  void TestLoopDescription();

  /** The outputs have the requested length, also for odd and tiny loops. */
  void TestLoopLength();

  /** The amplitude is 1 + depth * sin: start, extremes, mean, every frame. */
  void TestAmplitudeShape();

  /** A zero amplitude depth gives a flat 1.0. */
  void TestZeroAmplitudeDepth();

  /** The pitch is empty at 0 cents and of the full length otherwise. */
  void TestPitchPresence();

  /** Phase 0 follows the amplitude, pi/2 leads, pi is the negative. */
  void TestPitchPhase();

  /** Old longer vectors are overwritten, a stale pitch is emptied. */
  void TestOutputReuse();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTREMULANTPROVIDERSYNTH_H */
