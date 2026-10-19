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
 * GOSoundTremulantProvider: the period length, the amplitude and the pitch
 * deviations, the zero means and the channels that were requested.
 */
class GOTestSoundTremulantProviderSynth : public GOTest {
private:
  static const std::string TEST_NAME;

  /** The deviations have the requested length, also for odd and tiny
   * periods. */
  void TestPeriodLength();

  /** The amplitude is sin: start, extremes, mean, every frame; and it equals
   * the pitch deviation. */
  void TestAmplitudeShape();

  /** Both deviations have the zero mean at any period, also where the period
   * covers only part of the sine's shape. */
  void TestZeroMean();

  /** A deviation that is not needed is empty, a needed one is the same as
   * when both are needed. */
  void TestNeededChannels();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTREMULANTPROVIDERSYNTH_H */
