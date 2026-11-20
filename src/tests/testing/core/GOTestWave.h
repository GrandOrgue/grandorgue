/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTWAVE_H
#define GOTESTWAVE_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOWave::ReadSamples() on RIFF/WAVE files built in memory: the
 * decoding of 16/24-bit PCM and 32-bit float.
 */
class GOTestWave : public GOTest {
private:
  static const std::string TEST_NAME;

  /** 16- and 24-bit PCM read in their own format come out unchanged */
  void TestIntegersUnchanged();

  /** A float file read as float stays bit-exact */
  void TestFloatBitExact();

  /**
   * Selecting one channel of a stereo float file and its downmix to mono are
   * computed in float, without rounding the samples to integers
   */
  void TestFloatChannelSelectionAndDownmix();

  /** A 32-bit integer PCM file is rejected: 4 bytes is not necessarily float */
  void TestThirtyTwoBitPcmRejected();
  void TestOwnFormat();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTWAVE_H */
