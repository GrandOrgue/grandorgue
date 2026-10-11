/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDLINEARFADERCORE_H
#define GOTESTPERFSOUNDLINEARFADERCORE_H

#include "../buffer/GOTestPerfSoundBufferBase.h"

#include <string>

/**
 * Measures the throughput of GOSoundLinearFaderCore::fade() and crossfade()
 * (Mitems/sec). Every iteration refills the data first (a repeated in-place
 * fade would drive the data to denormals); the refill is a part of the
 * measured time.
 */
class GOTestPerfSoundLinearFaderCore : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  /** Fewer frames than the default (a tenth of the former 1000000 iterations
   * per buffer size): the throughput does not depend on the number of
   * frames, and the default made this suite one of the slowest in
   * GOTestExe. */
  unsigned GetNFrames() const override { return 64000000; }

  /** fade<2>, the piece entirely inside the ramp (the playing fader's path). */
  void TestPerfFadeStereoInside();

  /** fade<2>, the piece half inside the ramp, half past its end. */
  void TestPerfFadeStereoHalf();

  /** fade<2>, the piece entirely past the ramp end (end gain 0.5). */
  void TestPerfFadeStereoPast();

  /** fade<2> with a constant-gain ramp of the gain 1 (nothing to do). */
  void TestPerfFadeStereoConstOne();

  /** fade<2> with a constant-gain ramp of the gain 0 (fill). */
  void TestPerfFadeStereoConstZero();

  /** fade<1> on a mono buffer, inside the ramp. */
  void TestPerfFadeMono();

  /** fadePlanar: one channel inside the ramp, two channels inside, half
   * inside and past the ramp end. */
  void TestPerfFadePlanar();

  /** crossfade<2> on interleaved stereo: inside, half, and entirely past the
   * ramp end for TO_OTHER (copy) and FROM_OTHER (nothing). */
  void TestPerfCrossfadeInterleaved();

  /** crossfadePlanar on one and two channels, entirely inside the ramp. */
  void TestPerfCrossfadePlanarInside();

  /** crossfadePlanar entirely past the ramp end: TO_OTHER (copy) and
   * FROM_OTHER (nothing). */
  void TestPerfCrossfadePlanarPast();

  /** crossfadePlanar, half inside the ramp, half past its end. */
  void TestPerfCrossfadePlanarHalf();

public:
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  void run() override;
  bool tearDown() override;
};

#endif /* GOTESTPERFSOUNDLINEARFADERCORE_H */
