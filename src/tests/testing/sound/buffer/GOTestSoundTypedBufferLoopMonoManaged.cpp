/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTypedBufferLoopMonoManaged.h"

#include <cstdint>
#include <string>

#include "sound/buffer/GOSoundTypedBufferLoopMonoManaged.h"

#include "GOTestScope.h"

const std::string GOTestSoundTypedBufferLoopMonoManaged::TEST_NAME
  = "GOTestSoundTypedBufferLoopMonoManaged";

using LoopPosition = GOSoundTypedBufferLoopBase::LoopPosition;

/** Fills the loop with 1, 2, 3, ... and the tail */
template <typename T>
static void fill_sequential(GOSoundTypedBufferLoopMonoManaged<T> &buffer) {
  T *pLoop = buffer.GetLoopData();

  for (unsigned frameI = 0, n = buffer.GetNLoopFrames(); frameI < n; ++frameI)
    pLoop[frameI] = (T)(frameI + 1);
  buffer.FillTail();
}

void GOTestSoundTypedBufferLoopMonoManaged::TestEmpty() {
  GOSoundTypedBufferLoopMonoManaged<uint16_t> buffer;

  GOAssert(!buffer.IsValid(), "a default constructed buffer is empty");
  GOAssert(
    buffer.GetNLoopFrames() == 0 && buffer.GetNChunkFrames() == 0,
    "an empty buffer has zero sizes");

  buffer.Resize(4, 3);
  GOAssert(buffer.IsValid(), "a resized buffer is not empty");
  GOAssert(
    buffer.GetNLoopFrames() == 4 && buffer.GetNChunkFrames() == 3,
    "Resize() sets the sizes");

  buffer = {};
  GOAssert(!buffer.IsValid(), "assigning an empty buffer empties it");
}

void GOTestSoundTypedBufferLoopMonoManaged::TestChunkAcrossSeam() {
  static constexpr unsigned N_LOOP_FRAMES = 7;
  static constexpr unsigned N_CHUNK_FRAMES = 4;

  GOSoundTypedBufferLoopMonoManaged<uint16_t> buffer(
    N_LOOP_FRAMES, N_CHUNK_FRAMES);
  LoopPosition pos(N_LOOP_FRAMES);

  fill_sequential(buffer);
  for (unsigned stepI = 0; stepI < 2 * N_LOOP_FRAMES; ++stepI) {
    const uint16_t *pChunk = buffer.GetChunk(pos);

    for (unsigned frameI = 0; frameI < N_CHUNK_FRAMES; ++frameI)
      GOAssert(
        pChunk[frameI]
          == (uint16_t)((pos.GetIndex() + frameI) % N_LOOP_FRAMES + 1),
        "step " + std::to_string(stepI) + ", frame " + std::to_string(frameI)
          + ": the chunk must equal the cyclic read of the loop");
    pos.Advance(1);
  }
}

void GOTestSoundTypedBufferLoopMonoManaged::TestTiledTail() {
  static constexpr unsigned N_CHUNK_FRAMES = 9;

  for (unsigned nLoopFrames = 1; nLoopFrames <= 3; ++nLoopFrames) {
    GOSoundTypedBufferLoopMonoManaged<uint16_t> buffer(
      nLoopFrames, N_CHUNK_FRAMES);
    LoopPosition pos(nLoopFrames);

    fill_sequential(buffer);
    for (unsigned stepI = 0; stepI < nLoopFrames; ++stepI) {
      const uint16_t *pChunk = buffer.GetChunk(pos);

      for (unsigned frameI = 0; frameI < N_CHUNK_FRAMES; ++frameI)
        GOAssert(
          pChunk[frameI]
            == (uint16_t)((pos.GetIndex() + frameI) % nLoopFrames + 1),
          "loop " + std::to_string(nLoopFrames) + ", step "
            + std::to_string(stepI) + ", frame " + std::to_string(frameI)
            + ": the tiled tail must repeat the loop");
      pos.Advance(1);
    }
  }
}

void GOTestSoundTypedBufferLoopMonoManaged::TestSharedPositionDifferentTypes() {
  static constexpr unsigned N_LOOP_FRAMES = 5;
  static constexpr unsigned N_CHUNK_FRAMES = 3;

  GOSoundTypedBufferLoopMonoManaged<uint16_t> intBuffer(
    N_LOOP_FRAMES, N_CHUNK_FRAMES);
  GOSoundTypedBufferLoopMonoManaged<float> floatBuffer(
    N_LOOP_FRAMES, N_CHUNK_FRAMES);
  LoopPosition pos(N_LOOP_FRAMES);

  fill_sequential(intBuffer);
  fill_sequential(floatBuffer);
  for (unsigned stepI = 0; stepI < N_LOOP_FRAMES + 2; ++stepI) {
    const uint16_t *pIntChunk = intBuffer.GetChunk(pos);
    const float *pFloatChunk = floatBuffer.GetChunk(pos);

    for (unsigned frameI = 0; frameI < N_CHUNK_FRAMES; ++frameI)
      GOAssert(
        (float)pIntChunk[frameI] == pFloatChunk[frameI],
        "step " + std::to_string(stepI) + ", frame " + std::to_string(frameI)
          + ": one position must give the same phase in both buffers");
    pos.Advance(2);
  }
}

void GOTestSoundTypedBufferLoopMonoManaged::TestMove() {
  GOSoundTypedBufferLoopMonoManaged<uint16_t> source(4, 2);

  fill_sequential(source);

  GOSoundTypedBufferLoopMonoManaged<uint16_t> moved(std::move(source));
  const LoopPosition pos(4);

  GOAssert(!source.IsValid(), "the source of a move is empty");
  GOAssert(
    moved.GetNLoopFrames() == 4 && moved.GetNChunkFrames() == 2,
    "the move constructor takes the sizes");
  GOAssert(
    moved.GetChunk(pos)[0] == 1 && moved.GetChunk(pos)[1] == 2,
    "the move constructor takes the data");

  GOSoundTypedBufferLoopMonoManaged<uint16_t> assigned;

  assigned = std::move(moved);
  GOAssert(!moved.IsValid(), "the source of a move assignment is empty");
  GOAssert(
    assigned.GetNLoopFrames() == 4 && assigned.GetChunk(pos)[1] == 2,
    "the move assignment takes the data");
}

void GOTestSoundTypedBufferLoopMonoManaged::TestResizeToZero() {
  GOSoundTypedBufferLoopMonoManaged<uint16_t> buffer(8, 4);

  GOAssert(buffer.IsValid(), "a constructed buffer is not empty");
  buffer.Resize(0, 0);
  GOAssert(!buffer.IsValid(), "Resize(0, 0) empties the buffer");
  GOAssert(
    buffer.GetNLoopFrames() == 0 && buffer.GetNChunkFrames() == 0,
    "an emptied buffer has zero sizes");

  buffer.Resize(8, 4);
  GOAssert(
    buffer.IsValid() && buffer.GetNLoopFrames() == 8
      && buffer.GetNChunkFrames() == 4,
    "an emptied buffer can be resized again");
}

void GOTestSoundTypedBufferLoopMonoManaged::run() {
  GO_RUN_TEST(TestEmpty())
  GO_RUN_TEST(TestResizeToZero())
  GO_RUN_TEST(TestChunkAcrossSeam())
  GO_RUN_TEST(TestTiledTail())
  GO_RUN_TEST(TestSharedPositionDifferentTypes())
  GO_RUN_TEST(TestMove())
}
