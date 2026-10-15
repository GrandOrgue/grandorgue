/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTYPEDBUFFERLOOPMONOMANAGED_H
#define GOSOUNDTYPEDBUFFERLOOPMONOMANAGED_H

#include <algorithm>
#include <memory>
#include <utility>

#include "GOSoundTypedBufferLoopBase.h"

/**
 * The owner of a mono loop buffer with a tail (see GOSoundTypedBufferLoopBase).
 * Move-only. Filled in three steps, on a non-audio thread:
 *   1. Resize(nLoopFrames, nChunkFrames) allocates;
 *   2. the caller writes nLoopFrames values through GetLoopData();
 *   3. FillTail() copies the start of the loop into the tail.
 * There is deliberately no "build from a loop array": callers compute values
 * straight into the buffer instead of through a loop-sized temporary.
 * Emptying is by Resize(0, 0) or by move assignment of an empty object:
 * `buffer = {};`.
 *
 * Immutable after FillTail() as far as readers are concerned, so any number of
 * threads may call GetChunk() at once; each thread needs its own LoopPosition.
 *
 * Reserved for the buffer-family refactoring (not implemented): the read-only
 * view GOSoundTypedBufferLoopMono<T> that this class will derive from, and the
 * interleaved GOSoundTypedBufferLoop<T> and planar
 * GOSoundTypedBufferPlanarLoop<T>. GetChunk() will then return a
 * GOSoundTypedBufferMono<T> instead of a pointer.
 *
 * @tparam T the type of one value (for example uint16_t or float)
 */
template <typename T>
class GOSoundTypedBufferLoopMonoManaged : public GOSoundTypedBufferLoopBase {
private:
  /** nLoopFrames + nChunkFrames - 1 values: the loop followed by the tail. */
  std::unique_ptr<T[]> o_data;
#ifndef NDEBUG
  /** Debug only: true between FillTail() and the next change of the data. */
  bool m_IsTailFilled = false;
#endif

public:
  GOSoundTypedBufferLoopMonoManaged() = default;

  /** Allocates like Resize(); the content is undefined. */
  GOSoundTypedBufferLoopMonoManaged(
    unsigned nLoopFrames, unsigned nChunkFrames) {
    Resize(nLoopFrames, nChunkFrames);
  }

  GOSoundTypedBufferLoopMonoManaged(const GOSoundTypedBufferLoopMonoManaged &)
    = delete;
  GOSoundTypedBufferLoopMonoManaged &operator=(
    const GOSoundTypedBufferLoopMonoManaged &)
    = delete;

  /** Takes the data over; other becomes empty. */
  GOSoundTypedBufferLoopMonoManaged(
    GOSoundTypedBufferLoopMonoManaged &&other) noexcept {
    *this = std::move(other);
  }

  /** Takes the data over and frees the old; other becomes empty. */
  GOSoundTypedBufferLoopMonoManaged &operator=(
    GOSoundTypedBufferLoopMonoManaged &&other) noexcept {
    if (this != &other) {
      o_data = std::move(other.o_data);
      m_NLoopFrames = std::exchange(other.m_NLoopFrames, 0);
      m_NChunkFrames = std::exchange(other.m_NChunkFrames, 0);
#ifndef NDEBUG
      m_IsTailFilled = std::exchange(other.m_IsTailFilled, false);
#endif
    }
    return *this;
  }

  /**
   * Allocates nLoopFrames + nChunkFrames - 1 values. The content is undefined
   * and the tail is not filled. Not for the audio thread (allocates and
   * frees). Overflow of the size is the caller's concern.
   * @param nLoopFrames the length of the loop; positive, or 0 to free the data
   *   and make the buffer empty (IsValid() is false)
   * @param nChunkFrames the length of the chunks; positive, or 0 together with
   *   nLoopFrames
   */
  void Resize(unsigned nLoopFrames, unsigned nChunkFrames) {
    assert((nLoopFrames > 0) == (nChunkFrames > 0));
    o_data = nLoopFrames
      ? std::make_unique<T[]>(size_t(nLoopFrames) + nChunkFrames - 1)
      : nullptr;
    m_NLoopFrames = nLoopFrames;
    m_NChunkFrames = nChunkFrames;
#ifndef NDEBUG
    m_IsTailFilled = false;
#endif
  }

  /**
   * @return the loop's nLoopFrames values to be written by the caller. Taking
   *   it invalidates the tail: call FillTail() afterwards.
   */
  T *GetLoopData() {
    assert(IsValid());
#ifndef NDEBUG
    m_IsTailFilled = false;
#endif
    return o_data.get();
  }

  /**
   * Copies the start of the loop into the tail; tiles it when the loop is
   * shorter than the tail (a one-frame loop gives a constant tail).
   */
  void FillTail() {
    assert(IsValid());

    const T *pLoop = o_data.get();
    T *pTail = o_data.get() + m_NLoopFrames;

    /* Copy the loop from its start as many times as the tail needs; only
     * the last piece may be shorter than the loop */
    for (unsigned nLeft = m_NChunkFrames - 1; nLeft;) {
      const unsigned nPiece = std::min(nLeft, m_NLoopFrames);

      std::copy(pLoop, pLoop + nPiece, pTail);
      pTail += nPiece;
      nLeft -= nPiece;
    }
#ifndef NDEBUG
    m_IsTailFilled = true;
#endif
  }

  /**
   * Returns nChunkFrames consecutive values starting at the position, across
   * the seam of the loop if needed. No copy is made.
   * @param pos the position; its loop length must equal this buffer's
   * @return the first value of the chunk; valid until the buffer is resized,
   *   moved from or destroyed
   */
  const T *GetChunk(const LoopPosition &pos) const {
    assert(IsValid());
    assert(m_IsTailFilled);
    assert(pos.IsValid() && pos.GetNLoopFrames() == m_NLoopFrames);
    return o_data.get() + pos.GetIndex();
  }
};

#endif /* GOSOUNDTYPEDBUFFERLOOPMONOMANAGED_H */
