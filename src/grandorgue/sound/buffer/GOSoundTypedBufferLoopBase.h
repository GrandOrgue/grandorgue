/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTYPEDBUFFERLOOPBASE_H
#define GOSOUNDTYPEDBUFFERLOOPBASE_H

#include <cassert>
#include <climits>
#include <cstdint>

/**
 * The type-independent part of a loop buffer: a buffer that holds one period
 * of a periodic function (the loop) followed by a tail that repeats the start
 * of the loop. The tail makes any chunk of nChunkFrames consecutive values
 * contiguous in memory even when it crosses the seam of the loop, so a reader
 * can hand the chunk to code that knows nothing about the loop.
 *
 * It holds only the sizes and defines the LoopPosition, so one LoopPosition can
 * walk several buffers of different value types that have the same loop length
 * (for example the amplitude and the pitch of one tremulant, so that their
 * phase relation is exact by construction).
 */
class GOSoundTypedBufferLoopBase {
public:
  /**
   * A cursor in the loop, counted in frames. It is not bound to a buffer: it
   * stores its own loop length, which a buffer checks against its own in
   * GetChunk(). Not synchronised: it belongs to one thread at a time. Named
   * LoopPosition (not Position) because the future non-looped buffers will
   * have a plain Position, which a same-named nested class of a derived
   * looped buffer would hide.
   */
  class LoopPosition {
  private:
    /** The length of the loop in frames; 0 for an invalid position. */
    unsigned m_NLoopFrames = 0;
    /** The index of the next frame, below m_NLoopFrames (0 if invalid). */
    unsigned m_index = 0;

  public:
    /**
     * Creates an invalid position (no loop); the state of a cursor before the
     * loop it walks is known. Assignable, so it can be replaced.
     */
    inline LoopPosition() = default;

    /**
     * Creates a position at the start of a loop.
     * @param nLoopFrames the length of the loop; must be positive (a position
     *   without a loop is made by the default constructor)
     */
    inline explicit LoopPosition(unsigned nLoopFrames)
      : m_NLoopFrames(nLoopFrames) {
      assert(nLoopFrames);
    }

    /**
     * Creates a position in a loop of another length with the same relative
     * phase: index = old.index * nLoopFrames / old.nLoopFrames (the error is
     * at most one frame). Used when the loop is replaced by one with a
     * different length: `m_position = LoopPosition(m_position, newLen);`.
     * If the old position is invalid, the index is 0.
     * @param old the position to take the phase from
     * @param nLoopFrames the length of the new loop; must be positive
     */
    inline LoopPosition(const LoopPosition &old, unsigned nLoopFrames)
      : m_NLoopFrames(nLoopFrames),
        m_index(
          old.m_NLoopFrames ? (
            unsigned)(uint64_t(old.m_index) * nLoopFrames / old.m_NLoopFrames)
                            : 0) {
      assert(nLoopFrames);
    }

    /**
     * @return true if the position belongs to a loop and points into it
     *   (m_index < m_NLoopFrames; this implies a positive length); false for
     *   a default constructed one. Advance() and
     *   GOSoundTypedBufferLoopMonoManaged::GetChunk() require a valid position.
     */
    inline bool IsValid() const { return m_index < m_NLoopFrames; }

    inline unsigned GetNLoopFrames() const { return m_NLoopFrames; }

    /** @return the index of the current frame within the loop */
    inline unsigned GetIndex() const { return m_index; }

    /**
     * Moves forward, wrapping around the loop. nFrames may exceed the loop
     * length (a loop shorter than an audio period). Requires a valid position.
     * Called by the tremulant task on the audio thread in every audio period.
     * @param nFrames how many frames to advance
     */
    inline void Advance(unsigned nFrames) {
      assert(IsValid());
      /* nFrames is an audio period (at most MAX_FRAME_SIZE), the index is
       * below the loop length, so the sum fits an unsigned */
      assert(nFrames <= UINT_MAX - m_index);
      m_index += nFrames;
      if (m_index >= m_NLoopFrames) {
        /* Usually the loop is longer than an audio period, so one
         * subtraction wraps the index */
        m_index -= m_NLoopFrames;
        /* A loop not longer than an audio period (a short Period with a large
         * buffer or a low sample rate) is rare, so the division is fine */
        if (m_index >= m_NLoopFrames)
          m_index %= m_NLoopFrames;
      }
      assert(IsValid());
    }

    /** Returns to the start of the loop; the loop length is kept. */
    inline void Reset() { m_index = 0; }
  };

protected:
  /** The length of the loop in frames; 0 when empty. */
  unsigned m_NLoopFrames = 0;
  /**
   * The length of the chunks a reader takes; 0 when empty. The tail is one
   * frame shorter.
   */
  unsigned m_NChunkFrames = 0;

public:
  inline unsigned GetNLoopFrames() const { return m_NLoopFrames; }
  inline unsigned GetNChunkFrames() const { return m_NChunkFrames; }

  /**
   * @return true if there is a loop (false when default constructed or moved
   *   from)
   */
  inline bool IsValid() const { return m_NLoopFrames; }
};

#endif /* GOSOUNDTYPEDBUFFERLOOPBASE_H */
