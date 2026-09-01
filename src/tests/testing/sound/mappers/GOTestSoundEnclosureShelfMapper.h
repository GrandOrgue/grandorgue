/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDENCLOSURESHELFMAPPER_H
#define GOTESTSOUNDENCLOSURESHELFMAPPER_H

#include <string>

#include "GOTest.h"

class GOSoundShelfFilterProcessor;

/**
 * Exercises GOSoundEnclosureShelfMapper's caching contract: each band's
 * coefficients are recomputed if and only if something relevant to it
 * changed since the last EnsureParametersUpToDate() call. Friended by
 * GOSoundShelfFilterProcessor (matching GOTestSoundShelfFilterProcessor's
 * own direct-access pattern) so each case can stomp a band's Coeffs.b0 to
 * an impossible canary value before the call under test - this is what
 * lets the test tell "did not recompute" apart from "recomputed and
 * happened to land on the same value", which comparing final Coeffs alone
 * cannot do.
 */
class GOTestSoundEnclosureShelfMapper : public GOTest {
private:
  static const std::string TEST_NAME;

  // The field-touching helpers below are members (not free functions) only
  // because friend class GOSoundShelfFilterProcessor::m_LowCoeffs/
  // m_HighCoeffs access is granted to this class by name, not to arbitrary
  // functions in this .cpp file.

  /** Stomps the canary onto whichever band(s) are requested, before the
   * call under test. */
  void StompCanary(
    GOSoundShelfFilterProcessor &processor, bool stompLow, bool stompHigh);

  /** Asserts that isLowBand's Coeffs.b0 is still the canary - i.e. that
   * band was not recomputed since the last StompCanary() call. */
  void AssertBandNotRecomputed(
    const GOSoundShelfFilterProcessor &processor,
    bool isLowBand,
    const std::string &caseName);

  /** Asserts that isLowBand's Coeffs was recomputed (canary gone) and
   * matches what GOSoundOnePoleFilter::computeCoeffs() independently
   * produces for expectedFrequency/expectedGainDb. */
  void AssertBandRecomputed(
    const GOSoundShelfFilterProcessor &processor,
    bool isLowBand,
    double expectedFrequency,
    double expectedGainDb,
    const std::string &caseName);

  /** The first EnsureParametersUpToDate() call on a freshly constructed
   * mapper always pushes both bands, even though the enclosure itself was
   * never touched - the m_LastMidiValue == -1 sentinel forces it. */
  void TestFirstCallAlwaysPushesBothBands();

  /** A second call with nothing changed since the (already-synced)
   * baseline recomputes neither band. */
  void TestNothingChangedRecomputesNeitherBand();

  /** Changing only LowShelfFrequency recomputes the low band only. */
  void TestOnlyLowShelfFrequencyChanged();

  /** Changing only LowShelfAttenuationDb recomputes the low band only. */
  void TestOnlyLowShelfAttenuationDbChanged();

  /** Changing only HighShelfFrequency recomputes the high band only. */
  void TestOnlyHighShelfFrequencyChanged();

  /** Changing only HighShelfAttenuationDb recomputes the high band only. */
  void TestOnlyHighShelfAttenuationDbChanged();

  /** Changing only the enclosure's MIDI position recomputes both bands,
   * since both GetCurrentLowShelfGainDb()/GetCurrentHighShelfGainDb()
   * depend on position. */
  void TestOnlyPositionChanged();

  /** Changing position and one band's config in the same round recomputes
   * both bands (position alone already forces both), and the changed
   * band's pushed frequency/gain reflect the new config, not the old one. */
  void TestPositionAndOneBandConfigChangedTogether();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDENCLOSURESHELFMAPPER_H */
