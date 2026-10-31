/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDVIBRATOPITCHRATECURVE_H
#define GOTESTPERFSOUNDVIBRATOPITCHRATECURVE_H

#include "../buffer/GOTestPerfSoundBufferBase.h"

#include <cstdint>
#include <string>
#include <vector>

/**
 * Measures GOSoundVibratoPitchRateCurve: applyAffineTransform(),
 * normalisePitchRateCurve() (a plain shift and the pressing), Build() with and
 * without the normalisation, and the chunk lookup. The unit is Mcells/s of
 * the curve (the curve length plays the role of the buffer size) except for the
 * cursor, which is measured in Mperiods/s. The functions that work in place are
 * measured together with a copy of the master data, so TestPerfCopy() gives the
 * reference.
 */
class GOTestPerfSoundVibratoPitchRateCurve : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  /** The number of frames (curve cells) of the case being measured; set per
   * case so that a case does about the same total work whatever the curve
   * length. */
  unsigned m_NFrames = 1000;

  unsigned GetNFrames() const override { return m_NFrames; }

  /**
   * Builds a not normalised curve of curveLen cells from the frequency
   * deviations and returns its rates (without the tail): the master data of
   * the in-place tests.
   */
  static std::vector<uint16_t> makeMasterRates(
    unsigned curveLen, const std::vector<float> &deviations);

  /** applyAffineTransform() alone with k = 1/2. */
  void TestPerfApplyAffineTransform();

  /** normalisePitchRateCurve(), the plain shift with a non-zero remainder. */
  void TestPerfNormaliseShift();

  /** normalisePitchRateCurve(), the pressing of a deep square wave. */
  void TestPerfNormalisePressed();

  /** BuildInternal(..., false): allocation, exp2 and the chunk sums. */
  void TestPerfBuildNotNormalised();

  /** Build(): the same plus the normalisation. */
  void TestPerfBuildNormalised();

  /** The copy of the master data alone: the reference for the in-place tests.
   */
  void TestPerfCopy();

  /** GetChunkDescription() plus LoopPosition::Advance(), once per period. */
  void TestPerfChunkDescriptionAdvance();

public:
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  void run() override;
  bool tearDown() override;
};

#endif /* GOTESTPERFSOUNDVIBRATOPITCHRATECURVE_H */
