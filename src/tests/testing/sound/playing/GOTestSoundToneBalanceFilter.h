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
 * (Init()/IsToApply()/SetSamplerate()) and its nested State
 * (Init()/IsToApply()/ProcessBuffer()), proving the Stage 5 restructure
 * (absorbing the former GOSoundOnePoleFilter instance API/FilterState) kept
 * the exact same observable behavior.
 */
class GOTestSoundToneBalanceFilter : public GOTest {
private:
  static const std::string TEST_NAME;

  /** value == 0 disables filtering (IsToApply() false); value < 0 enables
   * an LPF, value > 0 an HPF (IsToApply() true), until SetSamplerate() is
   * called again. */
  void TestInitAndIsToApply();

  /** SetSamplerate() resets the instance to the identity filter until
   * Init() is called again. */
  void TestSetSamplerateResetsToIdentity();

  /** State::IsToApply() is false when unbound (nullptr) or bound to a
   * no-op filter, and true once bound to a configured filter. */
  void TestStateIsToApply();

  /** State::ProcessBuffer() actually filters a bound, configured buffer,
   * and leaves an unfiltered (value == 0) buffer's samples equal to a
   * plain one-pole passthrough (i.e. unchanged, since the identity
   * Coeffs is a no-op) - callers are expected to skip the call entirely
   * when IsToApply() is false, so this only asserts the bound/configured
   * case actually changes the audio. */
  void TestStateProcessBufferFilters();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTONEBALANCEFILTER_H */
