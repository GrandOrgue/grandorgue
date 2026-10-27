/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDPLAYINGFADER_H
#define GOTESTPERFSOUNDPLAYINGFADER_H

#include "../buffer/GOTestPerfSoundBufferBase.h"

#include <string>

/**
 * Measures GOSoundPlayingFader::Process() throughput on interleaved stereo.
 * It is the baseline the refactoring of the fader onto GOSoundLinearFaderCore
 * must not regress. Every iteration starts from Setup(), so the measured
 * state is the same each time. The throughput is reported in Mitems/sec
 * (frames * 2 channels * the number of Process() calls of the iteration).
 */
class GOTestPerfSoundPlayingFader : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  /** A constant volume: the fast path without a ramp. */
  void TestPerfConstantGain();

  /** A fade-in over the whole block: the plain ramp path. */
  void TestPerfRamp();

  /** A growth turned into a decay with the crossing inside the second of two
   * blocks (two Process() calls per iteration). */
  void TestPerfGrowthToDecay();

  /** The target volume reached in the middle of a fade-in block. */
  void TestPerfTargetReachedMidBlock();

public:
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  void run() override;
  bool tearDown() override;
};

#endif /* GOTESTPERFSOUNDPLAYINGFADER_H */
