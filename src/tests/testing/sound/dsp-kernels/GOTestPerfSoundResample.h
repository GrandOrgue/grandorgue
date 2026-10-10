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
 * Measures GOSoundResample::ResampleBlockVariableRatePlanar()'s per-frame
 * throughput, for both LinearResampler and PolyphaseResampler.
 */
class GOTestPerfSoundResample : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  /** Fewer iterations than the base class default: this suite covers
   * several buffer-size/resampler combinations, and a full run across all
   * of them at the default iteration count would make it the slowest
   * test in GOTestExe. */
  unsigned GetNumIterations() const override { return 300000; }

  /** LinearResampler::ResampleBlockVariableRatePlanar(), stereo (the only
   * shape that exercises this path's per-channel position-trajectory
   * replay), fed a constant-value increment array - the planar output path
   * GOSoundVibratoProcessor::Process() will call. */
  void TestPerfResampleBlockVariableRatePlanarLinear();

  /** PolyphaseResampler::ResampleBlockVariableRatePlanar(), stereo, same
   * constant-increment-array setup as the Linear case above. */
  void TestPerfResampleBlockVariableRatePlanarPolyphase();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTPERFSOUNDRESAMPLE_H */
