/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTYPEDBUFFERLOOPBASE_H
#define GOTESTSOUNDTYPEDBUFFERLOOPBASE_H

#include "GOTest.h"

/** Tests of GOSoundTypedBufferLoopBase::LoopPosition. */
class GOTestSoundTypedBufferLoopBase : public GOTest {
private:
  static const std::string TEST_NAME;

  /** The constructors and IsValid() */
  void TestConstructors();
  /** Advance() wraps around the loop, also for steps longer than the loop */
  void TestAdvance();
  /** The (old, nLoopFrames) constructor keeps the relative phase */
  void TestRescalingConstructor();
  /** Reset() returns to the start and keeps the loop length */
  void TestReset();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTYPEDBUFFERLOOPBASE_H */
