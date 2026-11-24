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
 * decoding of 8-bit PCM and (later) of 16/24-bit PCM and 32-bit float.
 */
class GOTestWave : public GOTest {
private:
  static const std::string TEST_NAME;

  /** 8-bit PCM is offset binary: 0x00/0x80/0xFF/0x40 -> -128/0/+127/-64 */
  void TestEightBitSigned();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTWAVE_H */
