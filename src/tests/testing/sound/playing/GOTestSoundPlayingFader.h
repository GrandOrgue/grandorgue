/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDPLAYINGFADER_H
#define GOTESTSOUNDPLAYINGFADER_H

#include <string>

#include "GOTest.h"

/**
 * Pins the observable behaviour of GOSoundPlayingFader before it is
 * refactored onto GOSoundLinearFaderCore. One Process() call of n frames
 * applies ONE linear gain segment (a "chord") from the volume at the end of
 * the previous call to the volume computed for the end of this one; the shape
 * inside the block is lost. The tests check these chords, including the two
 * cases where the chord differs from the real shape: a growth turned into a
 * decay inside one block, and the target volume reached in the middle of a
 * block.
 *
 * All the volumes are chosen as dyadic fractions so the block-end values are
 * exact; the per-frame gains are compared within a small tolerance.
 */
class GOTestSoundPlayingFader : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A constant target volume gives a constant gain, also across calls. */
  void TestConstantGain();

  /** Setup() with nFramesToIncreaseIn gives a chord per block going up
   * from 0, continuous across blocks; IsSilent() is true at the start. */
  void TestFadeIn();

  /** StartDecreasingVolume() after a constant volume gives a decay chord per
   * block; the decay reaching 0 in the middle of a block gives the chord
   * to 0 over the whole block and IsSilent(). */
  void TestDecayToSilence();

  /** The external amplitude is reached by the end of the block (blocks of up
   * to 1024 frames) as one chord. */
  void TestExternalVolume();

  /** SetVelocityVolume() takes part in the external volume. */
  void TestVelocityVolume();

  /** A growth turned into a decay (StartDecreasingVolume() during an
   * unfinished fade-in): the crossing point lies exactly at the block end, in
   * the middle of the block, at the last frame of the block; the chord
   * cuts the peak off. */
  void TestGrowthToDecayInsideBlock();

  /** A growth turned into a decay whose decay reaches 0 in the block after
   * the turn: the chord goes to 0 over the whole block. */
  void TestGrowthToDecayReachingZero();

  /** The same growth-to-decay sequence cut into blocks of different sizes:
   * the shapes differ but the state after the same number of frames is
   * the same, and the gain never exceeds the original target or goes
   * negative. */
  void TestGrowthToDecayBlockCuts();

  /** The target volume reached in the middle of a fade-in block (no decay):
   * the chord goes to the target over the whole block, the next block is
   * constant. */
  void TestTargetReachedMidBlock();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDPLAYINGFADER_H */
