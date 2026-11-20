/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTTREMULANT_H
#define GOTESTTREMULANT_H

#include <string>

#include "GOTest.h"

/**
 * Exercises the parameters of GOTremulant: loading them from the ODF and the
 * .cmb, saving them to the .cmb, changing them by ApplySynthParamChanges() and
 * ResetSynthParamsToDefault(), and Render(), which turns them into the wave
 * form of the tremulant for a sample rate.
 */
class GOTestTremulant : public GOTest {
private:
  static const std::string TEST_NAME;

  /** StartRate and StopRate are converted to the ramp lengths in frames. */
  void TestLoadRateConversion();

  /** A missing Period, StartRate, StopRate or AmpModDepth throws. */
  void TestLoadRequiredKeys();

  /** The type is Synth by default, Wave is recognised. */
  void TestLoadTypes();

  /** Init() makes an internal Synth tremulant with the default parameters. */
  void TestInitDefaults();

  /** The .cmb gives the parameters of an internal tremulant, a missing key
   * gets its default. */
  void TestLoadFromCmbFallback();

  /** Init() does not mark the organ model as modified. */
  void TestInitDoesNotMarkModified();

  /** Save() writes all the parameters of an internal tremulant, which Init()
   * reads back. */
  void TestInitSaveRoundTrip();

  /** An ODF tremulant ignores the parameters in the .cmb and does not save
   * them. */
  void TestOdfIgnoresCmb();

  /** Loading does not mark the organ model as modified. */
  void TestLoadDoesNotMarkModified();

  /** ApplySynthParamChanges() marks the organ model as modified only if a
   * value changes. */
  void TestApplyMarksModified();

  /** The getters return what ApplySynthParamChanges() stored. */
  void TestGettersReturnSetValues();

  /** GOParamRange::Contains includes both ends and Clamp() keeps the values
   * inside them; the ranges of GOTremulant are what the .cmb accepts. */
  void TestParamRanges();

  /** ApplySynthParamChanges applies only the present values, reports whether
   * anything changed and marks the organ model as modified only then. */
  void TestApplySynthParamChanges();

  /** ApplySynthParamChanges clamps the out-of-range values to the ranges. */
  void TestApplyClampsToRanges();

  /** ResetSynthParamsToDefault gives the same parameters as Init() without
   * keys in the .cmb and reports whether anything changed. */
  void TestResetSynthParamsToDefault();

  /** The length of the period for several periods and sample rates. */
  void TestRenderPeriodLength();

  /** The depth of the amplitude modulation. */
  void TestRenderAmplitude();

  /** The pitch modulation: presence, depth and the reduction of the phase. */
  void TestRenderPitch();

  /** The depths and the peaks of the pitch modulation agree with the table of
   * the help. */
  void TestRenderPitchTable();

  /** The deepest pitch modulation reaches the lowest deviation the vibrato
   * curve accepts. */
  void TestRenderPitchDepthBound();

  /** A zero depth empties the shape of that modulation. */
  void TestRenderZeroDepth();

  /** Both shapes have the zero mean, so they do not shift the level or the
   * pitch on average. */
  void TestRenderZeroMean();

  /** ApplySynthParamChanges() changes what Render() produces. */
  void TestRenderUsesParams();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTTREMULANT_H */
