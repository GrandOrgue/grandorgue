/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDRESAMPLE_H
#define GOTESTPERFSOUNDRESAMPLE_H

#include "../buffer/GOTestPerfSoundBufferBase.h"

#include <string>

/**
 * Measures GOSoundResample::ScalarProductionResampler::ResampleBlock()'s
 * per-frame throughput, for both LinearResampler and PolyphaseResampler.
 * This is the baseline any future refactor of ResampleBlock() must not
 * regress.
 */
class GOTestPerfSoundResample : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  /** Fewer iterations than the base class default: this suite covers
   * several buffer-size/resampler combinations, and a full run across all
   * of them at the default iteration count would make it the slowest
   * test in GOTestExe. */
  unsigned GetNumIterations() const override { return 300000; }

  /** LinearResampler::ResampleBlock(), mono float source to float. */
  void TestPerfResampleBlockMonoFloatLinear();

  /** PolyphaseResampler::ResampleBlock(), mono float source to float. */
  void TestPerfResampleBlockMonoFloatPolyphase();

  /** LinearResampler::ResampleBlock(), stereo GOInt24 source to float,
   * constant rate 1.0 - the realistic decoded-pipe-sample shape
   * (GOSoundStream's own StreamPtrWindow<GOInt24, 2>), unlike the
   * mono-float synthetic cases above. */
  void TestPerfResampleBlockStereo24Linear();

  /** PolyphaseResampler::ResampleBlock(), stereo GOInt24 source to float,
   * constant rate 1.0. */
  void TestPerfResampleBlockStereo24Polyphase();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTPERFSOUNDRESAMPLE_H */
