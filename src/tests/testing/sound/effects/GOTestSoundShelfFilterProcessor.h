/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDSHELFFILTERPROCESSOR_H
#define GOTESTSOUNDSHELFFILTERPROCESSOR_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundShelfFilterProcessor/GOSoundShelfFilterProcessorState
 * (the windchest two-band shelf EQ built for GrandOrgue issue #717).
 * Friended by both classes for direct field access, matching sound/effects/'s
 * own GOTestSoundReverbProcessor.
 */
class GOTestSoundShelfFilterProcessor : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A freshly constructed processor leaves both bands not isActive (identity)
   * until SetLowShelf()/SetHighShelf() is called. */
  void TestConstructorLeavesBothBandsNoop();

  /** EnsureSetup() recomputes both bands' coefficients only when
   * sampleRate actually changes - a repeated call with the same
   * sampleRate must not reset a Coeffs a prior SetLowShelf()/SetHighShelf()
   * call configured. */
  void TestEnsureSetupRecomputesOnlyOnSampleRateChange();

  /** SetLowShelf()/SetHighShelf() each recompute only their own band's
   * coefficients, leaving the other band untouched. */
  void TestSettersAffectOnlyTheirOwnBand();

  /** CreateTypedState() asserts when called before EnsureSetup(); returns
   * a correctly-sized, zeroed state otherwise. */
  void TestCreateTypedStateSizing();

  /** Both bands not isActive leaves the buffer completely untouched. */
  void TestProcessBothBandsNoopIsBypass();

  /** A gain-0 band produces bit-identical output to input for that band;
   * only the configured band's state is touched. */
  void TestProcessOnlyConfiguredBandIsApplied();

  /** Low-then-high series application across mono and stereo channel
   * counts actually changes the audio when both bands are configured. */
  void TestProcessAppliesBothBandsInSeries();

  /** Calling SetLowShelf()/SetHighShelf() between two Process() calls
   * changes the second call's output immediately. */
  void TestDynamicUpdateTakesEffectImmediately();

  /** A band switched to gain 0 leaves the audio untouched and has its state
   * cleared, so re-enabling it later does not produce a transient from
   * stale memory (silent input stays exactly silent). */
  void TestBandStateClearedWhenBecomingInactive();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDSHELFFILTERPROCESSOR_H */
