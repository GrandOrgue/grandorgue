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
 * and ResampleBlockVariableRatePlanar()'s / ResampleBlockConstantRatePlanar()'s
 * per-frame throughput, for both LinearResampler and PolyphaseResampler.
 * The ResampleBlock() cases are the baseline any future refactor of it must
 * not regress.
 */
class GOTestPerfSoundResample : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  /** Fewer frames than the base class default: this suite covers several
   * buffer-size/resampler combinations, and a full run across all of them at
   * the default would make it the slowest test in GOTestExe. */
  unsigned GetNFrames() const override { return 20000000; }

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

  /** LinearResampler::ResampleBlockVariableRatePlanar(), stereo (the only
   * shape that exercises this path's per-channel position-trajectory
   * replay), fed a constant-value increment array - the planar output path
   * GOSoundVibratoProcessor::Process() will call. */
  void TestPerfResampleBlockVariableRatePlanarLinear();

  /** PolyphaseResampler::ResampleBlockVariableRatePlanar(), stereo, same
   * constant-increment-array setup as the Linear case above. */
  void TestPerfResampleBlockVariableRatePlanarPolyphase();

  /** LinearResampler::ResampleBlockConstantRatePlanar(), stereo, rate 1.0
   * with a non-zero starting fraction over a RingPlanarFrameVector (the
   * frozen-lag case of GOSoundVibratoProcessor). */
  void TestPerfResampleBlockConstantRatePlanarLinear();

  /** The same for PolyphaseResampler. */
  void TestPerfResampleBlockConstantRatePlanarPolyphase();

  /** The common body of the two constant-rate planar tests.
   * @param name the test name for the report
   * @param resampler the resampler to measure
   * @param baselines the baseline table: sizes and minimal Mitems/sec */
  template <class ResamplerT, size_t nBaselines>
  void run_constant_rate_planar(
    const char *name,
    const ResamplerT &resampler,
    const GOTestPerfSoundBufferBaseline (&baselines)[nBaselines]);

public:
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  void run() override;
  bool tearDown() override;
};

#endif /* GOTESTPERFSOUNDRESAMPLE_H */
