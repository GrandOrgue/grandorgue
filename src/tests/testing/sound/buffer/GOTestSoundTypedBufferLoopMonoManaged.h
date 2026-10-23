/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTYPEDBUFFERLOOPMONOMANAGED_H
#define GOTESTSOUNDTYPEDBUFFERLOOPMONOMANAGED_H

#include "GOTest.h"

/** Tests of GOSoundTypedBufferLoopMonoManaged. */
class GOTestSoundTypedBufferLoopMonoManaged : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A default constructed buffer is empty */
  void TestEmpty();
  /** Resize(0, 0) frees the data and the buffer can be resized again */
  void TestResizeToZero();
  /** A chunk across the seam equals the cyclic read of the loop */
  void TestChunkAcrossSeam();
  /** The tail is tiled when the loop is shorter than the tail */
  void TestTiledTail();
  /** One LoopPosition walks buffers of different value types */
  void TestSharedPositionDifferentTypes();
  /** The move constructor and assignment transfer the data */
  void TestMove();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTYPEDBUFFERLOOPMONOMANAGED_H */
