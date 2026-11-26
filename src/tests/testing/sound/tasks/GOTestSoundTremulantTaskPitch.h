/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTREMULANTTASKPITCH_H
#define GOTESTSOUNDTREMULANTTASKPITCH_H

#include <string>

#include "GOTest.h"

/**
 * Exercises the pitch rate chunk that GOSoundTremulantTask publishes for the
 * vibrato: at the full depth, during a depth ramp, with the shared cursor,
 * and when there is no pitch. It is a friend of the task and reads its state
 * directly.
 */
class GOTestSoundTremulantTaskPitch : public GOTest {
private:
  static const std::string TEST_NAME;

  /** At the full depth the chunk points into the curve of the form. */
  void TestPitchChunkFullDepth();

  /** During a ramp the chunk is the mix of the curve and the unit rate. */
  void TestPitchChunkRamp();

  /** The chunk always starts at the cursor of the round, over many rounds. */
  void TestPitchSharesCursor();

  /** A reversal mid-ramp keeps the chunk sums close. */
  void TestPitchReversal();

  /** There are no rates when idle, without pitch and for a Wave tremulant. */
  void TestPitchChunkEmpty();

  /** SwapWaveForm() drops the chunk that pointed into the old curve. */
  void TestSwapResetsPitchChunk();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTREMULANTTASKPITCH_H */
