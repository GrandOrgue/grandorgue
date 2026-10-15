/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTypedBufferLoopBase.h"

#include "sound/buffer/GOSoundTypedBufferLoopBase.h"

#include "GOTestScope.h"

const std::string GOTestSoundTypedBufferLoopBase::TEST_NAME
  = "GOTestSoundTypedBufferLoopBase";

using LoopPosition = GOSoundTypedBufferLoopBase::LoopPosition;

void GOTestSoundTypedBufferLoopBase::TestConstructors() {
  const LoopPosition invalid;
  const LoopPosition pos(10);

  GOAssert(!invalid.IsValid(), "a default constructed position is invalid");
  GOAssert(invalid.GetNLoopFrames() == 0, "an invalid position has no loop");
  GOAssert(pos.IsValid(), "a position of a loop is valid");
  GOAssert(pos.GetNLoopFrames() == 10, "the loop length is kept");
  GOAssert(pos.GetIndex() == 0, "a position starts at the loop start");

  const GOSoundTypedBufferLoopBase base;

  GOAssert(!base.IsValid(), "an empty base is invalid");
  GOAssert(
    base.GetNLoopFrames() == 0 && base.GetNChunkFrames() == 0,
    "an empty base has zero sizes");
}

void GOTestSoundTypedBufferLoopBase::TestAdvance() {
  LoopPosition pos(5);

  pos.Advance(3);
  GOAssert(pos.GetIndex() == 3, "advance inside the loop");
  pos.Advance(2);
  GOAssert(pos.GetIndex() == 0, "advance exactly to the loop end wraps to 0");
  pos.Advance(4);
  pos.Advance(4);
  GOAssert(pos.GetIndex() == 3, "advance across the seam");
  pos.Advance(0);
  GOAssert(pos.GetIndex() == 3, "advance by 0 does not move");
  pos.Advance(5 * 7 + 1);
  GOAssert(pos.GetIndex() == 4, "advance by more than the loop length");

  LoopPosition oneFrame(1);

  oneFrame.Advance(1000);
  GOAssert(oneFrame.GetIndex() == 0, "a one-frame loop stays at 0");
  GOAssert(oneFrame.IsValid(), "a one-frame loop stays valid");
}

void GOTestSoundTypedBufferLoopBase::TestRescalingConstructor() {
  LoopPosition old(100);

  old.Advance(50);

  const LoopPosition longer(old, 300);
  const LoopPosition shorter(old, 10);
  const LoopPosition same(old, 100);
  const LoopPosition fromInvalid(LoopPosition(), 7);

  GOAssert(
    longer.GetNLoopFrames() == 300 && longer.GetIndex() == 150,
    "the phase is kept when the loop gets longer");
  GOAssert(
    shorter.GetNLoopFrames() == 10 && shorter.GetIndex() == 5,
    "the phase is kept when the loop gets shorter");
  GOAssert(
    same.GetNLoopFrames() == 100 && same.GetIndex() == 50,
    "the same length keeps the index");
  GOAssert(
    fromInvalid.GetNLoopFrames() == 7 && fromInvalid.GetIndex() == 0
      && fromInvalid.IsValid(),
    "from an invalid position the index is 0");
}

void GOTestSoundTypedBufferLoopBase::TestReset() {
  LoopPosition pos(8);

  pos.Advance(5);
  pos.Reset();
  GOAssert(pos.GetIndex() == 0, "Reset() returns to the start");
  GOAssert(pos.GetNLoopFrames() == 8, "Reset() keeps the loop length");
}

void GOTestSoundTypedBufferLoopBase::run() {
  GO_RUN_TEST(TestConstructors())
  GO_RUN_TEST(TestAdvance())
  GO_RUN_TEST(TestRescalingConstructor())
  GO_RUN_TEST(TestReset())
}
