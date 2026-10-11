/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDLINEARFADERCORE_H
#define GOTESTSOUNDLINEARFADERCORE_H

#include <string>

#include "GOTest.h"
#include "sound/dsp-kernels/GOSoundLinearFaderCore.h"

/**
 * Tests GOSoundLinearFaderCore: the GainRamp, the CrossfadeRamp, fade(),
 * crossfade(), crossfadePlanar() and mapFrameOffset().
 */
class GOTestSoundLinearFaderCore : public GOTest {
private:
  using Core = GOSoundLinearFaderCore;
  using GainRamp = Core::GainRamp;

  static const std::string TEST_NAME;

  /** Default ramp, constructor normalisation, constant-gain ramps. */
  void TestGainRampConstruction();

  /** GetGainAt, GetGainInside, IsOverAt, GetNFramesInRamp. */
  void TestGainRampQueries();

  /** GetGainSumBefore equals the sum of GetGainAt, also past the ramp end. */
  void TestGainRampGainSum();

  /** IsGainInRange and GetGainOffset. */
  void TestGainOffset();

  /** fade<1> and fade<2>: values, pieces, the part past the ramp end. */
  void TestFade();

  /** fadePlanar equals fade<1> per channel. */
  void TestFadePlanar();

  /** The dispatch of the constant gain of fade(). */
  void TestFadeConstantDispatch();

  /** UnitRamp construction and IsRising(). */
  void TestUnitRamp();

  /** CrossfadeRamp construction. */
  void TestCrossfadeRamp();

  /** crossfade<1>, crossfade<2>, the part past the ramp end. */
  void TestCrossfade();

  /** crossfadePlanar equals crossfade<1> per channel. */
  void TestCrossfadePlanar();

  /** canMapFrameOffset and mapFrameOffset. */
  void TestMapFrameOffset();

  /** mapFrameOffsetOrJump: a target without frames gives 0 without assert. */
  void TestMapFrameOffsetOrJump();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDLINEARFADERCORE_H */
