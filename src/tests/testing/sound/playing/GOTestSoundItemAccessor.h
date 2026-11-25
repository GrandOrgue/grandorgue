/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDITEMACCESSOR_H
#define GOTESTSOUNDITEMACCESSOR_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundItemAccessor: reading and writing the items of every
 * format in int and float units, also at unaligned addresses.
 */
class GOTestSoundItemAccessor : public GOTest {
private:
  static const std::string TEST_NAME;

  /**
   * The 8-, 16-, 24- and 32-bit integer items of two channels are read back
   * in int and float units, and the neighbour items are not touched.
   */
  void TestIntegerRoundTrip();

  /**
   * The float items are stored bit-exactly, read in int units as Q23, and an
   * int written to a float format is converted from Q23.
   */
  void TestFloatRoundTrip();

  /** A float written to an integer format is truncated towards zero. */
  void TestIntegerTruncatesFloat();

  /** A float format works at an address that is not aligned to 4 bytes. */
  void TestUnalignedData();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDITEMACCESSOR_H */
