/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDREVERB_H
#define GOTESTSOUNDREVERB_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundReverb::loadIRData() (Stage 5's extraction from
 * GOSoundReverb::Setup()) against a small real WAV fixture
 * (resources/sound/reverb/test-ir.wav: 10 mono float32 samples, 0.1..1.0, at
 * 44100 Hz), pinning its standalone contract and protecting the extraction
 * from silently changing Setup()'s existing behavior for its
 * master-bus-reverb caller.
 */
class GOTestSoundReverb : public GOTest {
private:
  static const std::string TEST_NAME;

  /** gain is applied to every loaded sample. */
  void TestLoadIRDataAppliesGain();

  /** startOffset/len trim the loaded data to [startOffset, startOffset+len). */
  void TestLoadIRDataTrimsOffsetAndLen();

  /** delay (milliseconds in the config) becomes samples at the target
   * sample rate in the result. */
  void TestLoadIRDataComputesDelayFromMs();

  /** isDirect passes through unchanged. */
  void TestLoadIRDataPassesThroughIsDirect();

  /** A missing/invalid file throws wxString, same as Setup() expects to
   * catch. */
  void TestLoadIRDataThrowsOnMissingFile();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDREVERB_H */
