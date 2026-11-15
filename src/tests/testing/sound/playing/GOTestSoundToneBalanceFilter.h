/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTONEBALANCEFILTER_H
#define GOTESTSOUNDTONEBALANCEFILTER_H

#include <string>

#include "GOTest.h"

/**
 * Regression coverage for GOSoundToneBalanceFilter's instance API
 * (Init()/SetSamplerate(), via the provider) and its nested State
 * (Init()/EnsureCoeffs()/IsToApply()/ProcessBuffer()), proving the Stage 5
 * restructure (absorbing the former GOSoundOnePoleFilter instance
 * API/FilterState) kept the exact same observable behavior.
 */
class GOTestSoundToneBalanceFilter : public GOTest {
private:
  static const std::string TEST_NAME;

  /** value == 0 disables filtering (IsToApply() false); value < 0 enables
   * an LPF, value > 0 an HPF (IsToApply() true), as seen by a State that
   * follows the provider. */
  void TestInitAndIsToApply();

  /** SetSamplerate() keeps the last value and recomputes its coefficients for
   * the new sample rate; it does not fall back to an unfiltered set. */
  void TestSetSamplerateKeepsValue();

  /** SetSamplerate() publishes exactly one set of coefficients: the final one.
   * A reader never gets an intermediate unfiltered set. */
  void TestSetSamplerateRepublishesOnce();

  /** After EnsureCoeffs(), State::IsToApply() is false for a provider with no
   * tone balance or a no-op filter, and true for a configured filter. */
  void TestStateIsToApply();

  /** State::ProcessBuffer() actually filters a bound, configured buffer,
   * and leaves an unfiltered (value == 0) buffer's samples equal to a
   * plain one-pole passthrough (i.e. unchanged, since the identity
   * Coeffs is a no-op) - callers are expected to skip the call entirely
   * when IsToApply() is false, so this only asserts the bound/configured
   * case actually changes the audio. */
  void TestStateProcessBufferFilters();

  /** A State following a provider takes over the coefficients that are set by
   * Init() after the previous call, at the next EnsureCoeffs(); its carried
   * memory is cleared when the filter is switched on again. */
  void TestStateFollowsFilterChanges();

  /** EnsureCoeffs() changes nothing when the filter was not changed since the
   * previous call, in particular it keeps the carried memory. */
  void TestEnsureCoeffsIsIdempotent();

  /** A copy of an active State keeps its carried memory and keeps following the
   * same coefficients, so the copy and the original produce the same output
   * for the same input. */
  void TestStateCopyKeepsMemory();

  /** While another thread changes the filter in a loop, the coefficients that
   * a State uses are always one complete set that was written by Init() and
   * never a mix of two sets. */
  void TestStateNeverSeesTornCoefficients();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTONEBALANCEFILTER_H */
