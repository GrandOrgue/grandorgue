/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GOSOUNDRESAMPLE_H_
#define GOSOUNDRESAMPLE_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

#include "go_compiler.h"

/**
 * A stateless DSP kernel: this class provides algorithms for resampling
 * audio buffers
 * Now two algorithms are supported: Linear and Polyphase.
 * They calculate a next output sample based on a vector of a few continous
 * input samples.
 */

class GOSoundResample {
public:
  static constexpr unsigned POLYPHASE_POINTS = 8;
  static constexpr unsigned LINEAR_POINTS = 2;

  /**
   * The largest per-item input-vector length (VECTOR_LENGTH) of any
   * resampler this class offers. Computed via std::max() from
   * POLYPHASE_POINTS/LINEAR_POINTS rather than just naming whichever one
   * happens to be larger today, so this stays correct on its own if a
   * future resampler type changes which one that is. A caller that needs
   * one fixed margin regardless of which InterpolationType it ends up
   * using (e.g. a ring buffer's mirrored-tail length, which is
   * MAX_POINTS - 1 frames, sized before the interpolation choice is
   * necessarily fixed) should use this constant.
   */
  static constexpr unsigned MAX_POINTS
    = std::max(POLYPHASE_POINTS, LINEAR_POINTS);

  /**
   * The largest number of channels a frame vector supports. It is the maximum
   * of uint8_t, the type of the compile-time channel count of
   * FloatingFrameVector; the channel index passed to its Seek() is a
   * uint_fast8_t, which is at least as wide, so no index is ever truncated.
   */
  static constexpr unsigned MAX_N_CHANNELS
    = std::numeric_limits<uint8_t>::max();

  static constexpr unsigned UPSAMPLE_BITS = 13;
  static constexpr unsigned UPSAMPLE_FACTOR = 1 << UPSAMPLE_BITS;
  static constexpr unsigned UPSAMPLE_MASK = UPSAMPLE_FACTOR - 1;

  /**
   * The largest increment in 1/UPSAMPLE_FACTOR units that
   * ResamplingPosition::Inc(unsigned) accepts: the fraction of a position is
   * always below UPSAMPLE_FACTOR, so the fraction plus such an increment cannot
   * wrap around unsigned.
   */
  static constexpr unsigned MAX_FRACTION_INCREMENT
    = std::numeric_limits<unsigned>::max() - UPSAMPLE_MASK;

  /**
   * The same limit expressed as the largest resampling rate (source frames per
   * target frame) rateToFractionIncrement() accepts. A whole number of frames,
   * so it is exactly representable as float.
   */
  static constexpr unsigned MAX_RATE = MAX_FRACTION_INCREMENT >> UPSAMPLE_BITS;

  enum InterpolationType {
    GO_LINEAR_INTERPOLATION = 0,
    GO_POLYPHASE_INTERPOLATION = 1,
  };

  /**
   * Converts a resampling-rate multiplier (source frames per target frame,
   * e.g. 1.0 = no change, 2^(cents/1200) for a pitch shift) into the
   * 1/UPSAMPLE_FACTOR-unit increment ResamplingPosition::Inc(unsigned) and
   * Init() use. Rounds half up; the rate must be non-negative. It is constexpr
   * so that compile-time rate constants can be built with it.
   *
   * The result is identical to roundf(rate * UPSAMPLE_FACTOR). It is computed
   * in double, not in float: multiplying by the power of two UPSAMPLE_FACTOR is
   * exact in both, but adding 0.5f in float rounds the sum when its binade is
   * coarser than the binade of the product - e.g. for the float just below
   * 0.5 / UPSAMPLE_FACTOR the product 0.5 - 2^-25 plus 0.5f is a tie that
   * rounds to 1.0f, so the result would be 1 instead of 0. Float spacing also
   * reaches 1 from 2^23, where the sum itself rounds. In double the sum is
   * exact for every float rate that fits the result, so none of this happens.
   */
  static constexpr unsigned rateToFractionIncrement(float rate) {
    // A negative rate would be undefined behavior in the cast below, NaN fails
    // both comparisons; 0 is allowed (a rate below half a unit rounds to 0)
    assert(rate >= 0.0f && rate <= MAX_RATE);
    return (unsigned)((double)rate * UPSAMPLE_FACTOR + 0.5);
  }

  /**
   * The number of whole frames the index has advanced by after any sequence of
   * Inc() calls whose arguments total nUnits, starting from the given fraction
   * (the exact index growth, rounded down).
   * @param fraction the starting fraction in 1/UPSAMPLE_FACTOR units
   * @param nUnits the sum of the Inc() arguments in 1/UPSAMPLE_FACTOR units
   */
  static inline unsigned getIndexIncrementByUnits(
    unsigned fraction, uint64_t nUnits) {
    return (unsigned)((fraction + nUnits) >> UPSAMPLE_BITS);
  }

  /**
   * The smallest sum of Inc() arguments after which the index has advanced by
   * at least nFrames whole frames, starting from the given fraction. 0 for
   * nFrames == 0.
   * @param fraction the starting fraction in 1/UPSAMPLE_FACTOR units
   * @param nFrames the number of whole frames to reach
   */
  static inline uint64_t computeMinNUnitsToReach(
    unsigned fraction, unsigned nFrames) {
    return nFrames > 0 ? ((uint64_t)nFrames << UPSAMPLE_BITS) - fraction : 0;
  }

  /**
   * The largest sum of Inc() arguments after which the position is still at or
   * before index + nFrames (a zero fraction at index + nFrames is allowed).
   * @param fraction the starting fraction in 1/UPSAMPLE_FACTOR units
   * @param nFrames the number of whole frames; must be >= 1
   */
  static inline uint64_t computeMaxNUnitsToReach(
    unsigned fraction, unsigned nFrames) {
    assert(nFrames > 0);
    return ((uint64_t)nFrames << UPSAMPLE_BITS) - fraction;
  }

  /**
   * A position in the source sample stream for the current position in the
   * target stream. Because the numbers of source and target samples differ, the
   * source position may be not integer and has a fractional part
   */
  class ResamplingPosition {
  private:
    // an integer part
    unsigned m_index;
    // a fractional part in 1/UPSAMPLE_FACTOR units
    unsigned m_fraction;
    // Increment of the source position for one target sample in
    //   1/UPSAMPLE_FACTOR units
    unsigned m_FractionIncrement;

    /**
     * Advances the position by the increment with no checks. The preconditions
     * are the ones of Inc(unsigned): IsValid() and the increment not above
     * MAX_FRACTION_INCREMENT.
     * @param fractionIncrement the source-position advance in
     *   1/UPSAMPLE_FACTOR units
     */
    GO_FORCE_INLINE void Advance(unsigned fractionIncrement) {
      m_fraction += fractionIncrement;
      m_index += m_fraction >> UPSAMPLE_BITS;
      m_fraction &= UPSAMPLE_MASK;
    }

  public:
    inline unsigned GetIndex() const { return m_index; }
    inline void SetIndex(unsigned newIndex) { m_index = newIndex; }
    inline unsigned GetFraction() const { return m_fraction; }
    inline unsigned GetFractionIncrement() const { return m_FractionIncrement; }

    /**
     * @return whether the position satisfies the class invariants: the
     *   fraction is below UPSAMPLE_FACTOR and the constant increment does not
     *   exceed MAX_FRACTION_INCREMENT. A zero increment is valid. Used in
     *   asserts and tests.
     */
    inline bool IsValid() const {
      return m_fraction < UPSAMPLE_FACTOR
        && m_FractionIncrement <= MAX_FRACTION_INCREMENT;
    }

    /**
     * @return whether the source rate equals the target rate, i.e. the
     *   constant increment is exactly one source frame per target frame and
     *   GetResamplingFactor() is 1.0
     */
    inline bool IsSameRate() const {
      return m_FractionIncrement == UPSAMPLE_FACTOR;
    }

    /**
     * A resampling factor equals to (source length / target length) or to
     * (source sample rate / target sample rate)
     * @return the resampling factor
     */
    inline float GetResamplingFactor() const {
      return (float)m_FractionIncrement / UPSAMPLE_FACTOR;
    }

    void Init(
      float factor,
      unsigned startIndex = 0,
      const ResamplingPosition *pOld = nullptr);

    /**
     * Advance the position for the next target frame by an explicit
     * increment, for a time-varying resampling rate (e.g. vibrato). Unlike
     * Inc(), this does not use m_FractionIncrement, so it does not disturb
     * AvailableTargetSamples() or GetResamplingFactor() for streams that
     * also use the constant-rate path.
     * @param fractionIncrement the source-position advance for this one
     *   target frame, in 1/UPSAMPLE_FACTOR units; must not exceed
     *   MAX_FRACTION_INCREMENT, so that the sum with the fraction (which is
     *   below UPSAMPLE_FACTOR) does not wrap around unsigned.
     */
    GO_FORCE_INLINE void Inc(unsigned fractionIncrement) {
      assert(fractionIncrement <= MAX_FRACTION_INCREMENT);
      Advance(fractionIncrement);
    }

    /**
     * Advance the position for the next target frame using the stream's
     * constant resampling rate. Equivalent to Inc(m_FractionIncrement), but
     * with no per-call checks: the resampling functions validate the position
     * (IsValid()) once per block.
     */
    GO_FORCE_INLINE void Inc() { Advance(m_FractionIncrement); }

    /**
     * Calculates the target samples length from given source position to the
     * end index
     * @param endIndex the first index position after the last source sample
     * @result - the number of tatget samples can be received before the current
     *   position exceeds endIndex specified
     * Requires a non-zero constant increment (GetFractionIncrement() > 0): it
     * is the divisor.
     */
    inline unsigned AvailableTargetSamples(unsigned endIndex) {
      assert(m_FractionIncrement > 0);
      return m_index < endIndex ? unsigned(
               (
                 // for preventing owerflowing if endIndex >= 2**19 ~ 550000
                 uint64_t(endIndex - m_index) * UPSAMPLE_FACTOR
                 + m_FractionIncrement - 1 // for rounding up
                 - m_fraction)
               / m_FractionIncrement)
                                : 0;
    }
  };

  /**
   * Represents an abstract vector of continous input samples somethere in the
   * input stream. It has nChannels input channels (1 - mono, 2 - stereo)
   * It has two main methods:
   * - void Seek(unsigned index, uint_fast8_t channel) - sets the vector to the
   * index position in the input stream for the specified channel. The
   *   implementation may restrict moving the position only forward
   * - void NextItem() - returns the next sample of the same channel. The
   *   implementation must provide sufficient number of samples required by
   *   the certain resampling algorithm.
   *
   * These methods are not virtual for a better performance. Their
   * implementation is substituted inline from the subclasses by a resampler.
   */
  template <uint8_t nChannels> struct FloatingFrameVector {
    static constexpr uint8_t m_NChannels = nChannels;
  };

  /**
   * A vector of continous samples in a memory region referenced by a pointer.
   * SrcItemT - a type of one sample. Usually int8_t, int16_t, GOInt24, or
   * float
   * ResItemT - a type of one sample returned by NextItem(). Usually int or
   * float
   * nChannels - number of channels in the source stream
   *
   * If nChannels>1 it asumes that samples are interleaving for several channels
   *     - 0 left
   *     - 0 right
   *     - 1 left
   *     - 1 right
   *     - ...
   * For the performance reason this NextItem() does not check for the bounds.
   * The calling program must ensure that there are sufficient number of samples
   */
  template <class SrcItemT, class ResItemT, uint8_t nChannels>
  class PtrFrameVector : public FloatingFrameVector<nChannels> {
  private:
    // points to the first sample of the 0-channel in the input stream
    const SrcItemT *p_StartPtr;
    // points to the current sample in the vector
    const SrcItemT *p_CurrPtr;

  protected:
    /**
     * Checks that the current sample pointer is before the specified end
     * pointer
     * @param endPtr the end pointer
     * @return are there more samples in the vector
     */
    inline bool IsBefore(const SrcItemT *endPtr) const {
      return p_CurrPtr < endPtr;
    }

  public:
    /**
     * Construct the vector with some start pointer.
     * @param ptr a pointer to the first (0 left) sample
     */
    inline PtrFrameVector(const SrcItemT *ptr) : p_StartPtr(ptr) {}

    /**
     * Moves the current sample pointer to the specified position in the input
     * stream for the channel specified
     * @param ptr a pointer to the first (0 left) sample
     */
    GO_FORCE_INLINE void Seek(unsigned index, uint_fast8_t channel) {
      p_CurrPtr = p_StartPtr + nChannels * index + channel;
    }

    /**
     * Returns the next sample from the vector and moves the current sample
     * pointer to the next sample of the same channel
     * @return the sample
     */
    GO_FORCE_INLINE ResItemT NextItem() {
      ResItemT res = (ResItemT)*p_CurrPtr;
      p_CurrPtr += nChannels;
      return res;
    }

    /**
     * No-op: this vector addresses a flat, non-recycled memory region, so
     * ResamplingPosition's index never needs correcting. Exists only so
     * ResampleBlock()/ResampleBlockVariableRatePlanar() can call
     * fV.NormalizePosition(resamplingPos) unconditionally, resolved
     * statically per FrameVectorT with no runtime branch - see
     * RingPlanarFrameVector's own override below for the vector type that
     * actually needs this.
     */
    inline void NormalizePosition(ResamplingPosition &) const {}

    /**
     * No-op counterpart of RingPlanarFrameVector::AssertIncrementFits():
     * a flat region has no wrap limit on a single step.
     */
    inline void AssertIncrementFits(unsigned) const {}
  };

  /**
   * A vector of continous samples in memory with checking for bounds on
   * NextItem().
   * This checking reduces the performance dramatically so it is intended to use
   * only in not realtime cases (for example, on loading, but not when playing)
   */
  template <class SrcItemT, class ResItemT, uint8_t nChannels>
  class BoundedPtrFrameVector
    : public PtrFrameVector<SrcItemT, ResItemT, nChannels> {
  private:
    /**
     * Points to the end of the memory region
     */
    const SrcItemT *p_EndPtr;

  public:
    /**
     * Constructs the vector with the start pointer and the length of the
     * memory region
     * @param ptr - a pointer to the first sample in the region
     * @param len - a number of samples of each channels
     */
    inline BoundedPtrFrameVector(const SrcItemT *ptr, unsigned len)
      : PtrFrameVector<SrcItemT, ResItemT, nChannels>(ptr),
        p_EndPtr(ptr + nChannels * len) {}

    GO_FORCE_INLINE ResItemT NextItem() {
      return PtrFrameVector<SrcItemT, ResItemT, nChannels>::IsBefore(p_EndPtr)
        ? PtrFrameVector<SrcItemT, ResItemT, nChannels>::NextItem()
        : (ResItemT)0;
    }
  };

  /**
   * A read-only FloatingFrameVector view over a planar (channel-major) ring
   * buffer, for a source whose read position can advance at a varying,
   * non-integer rate and wrap around a bounded window of history — e.g. a
   * modulated delay line such as pitch vibrato, the motivating use case but
   * not a dependency of this class. Channel-major like GOSoundBufferPlanar,
   * but strided by this class's own (nRingFrames + nTailFrames), not
   * GOSoundBufferPlanar's nFrames: channel c starts at pRingStart +
   * c * (nRingFrames + nTailFrames).
   *
   * Read only via ResampleBlock()/ResampleBlockVariableRatePlanar()/
   * ResampleBlockConstantRatePlanar(), which
   * call NormalizePosition() every frame — Seek() only asserts the index is
   * already below nRingFrames, it does not wrap it.
   *
   * Maximum increment: NormalizePosition() wraps with a single subtract, so
   * one resampling step must advance the index by less than a whole ring.
   * With the step increment given in 1/UPSAMPLE_FACTOR units, that means
   * (increment >> UPSAMPLE_BITS) + 1 <= nRingFrames (the +1 is the carry
   * from the starting fraction). The caller must choose nRingFrames large
   * enough for the largest rate it will ever use; this is checked per step
   * by AssertIncrementFits() in Debug builds only.
   */
  template <class SrcItemT, class ResItemT> class RingPlanarFrameVector {
  private:
    const SrcItemT *p_RingStart;
    const SrcItemT *p_CurrPtr;
#ifndef NDEBUG
    const SrcItemT *p_EndPtr;
#endif
    unsigned m_NRingFrames;
    unsigned m_NTailFrames;

  public:
    /**
     * Number of channels this ring holds, given at construction rather
     * than as a template parameter, so a caller with a runtime channel
     * count (e.g. GOSoundVibratoProcessor, whose buffer's channel count is
     * not known until EnsureSetup()) does not need one
     * RingPlanarFrameVector instantiation per possible channel count.
     * Public, matching FloatingFrameVector::m_NChannels's own visibility -
     * ResampleBlock() reads fV.m_NChannels directly, on whichever
     * FrameVectorT it is instantiated with.
     */
    const unsigned m_NChannels;

    /**
     * @param pRingStart pointer to channel 0's physical sub-buffer.
     * @param nChannels number of channels in the ring, at most
     *   MAX_N_CHANNELS; channel c's base is
     *   pRingStart + c * (nRingFrames + nTailFrames). Runtime value, not
     *   compile-time - see m_NChannels.
     * @param nRingFrames the ring's logical frame count — the index passed
     *   to Seek() must already be below this; must be > 0.
     * @param nTailFrames length, in frames, of the mirrored tail
     *   immediately following the ring in each channel's sub-buffer; must
     *   be at least the reading resampler's VECTOR_LENGTH - 1: the read
     *   index is always below nRingFrames, so the last tap of a vector
     *   starting there is VECTOR_LENGTH - 1 frames past the ring's end.
     *   That tail is exactly what Seek()/NextItem() rely on to read across
     *   the ring's seam without special-casing the wraparound.
     */
    inline RingPlanarFrameVector(
      const SrcItemT *pRingStart,
      unsigned nChannels,
      unsigned nRingFrames,
      unsigned nTailFrames)
      : p_RingStart(pRingStart),
        m_NRingFrames(nRingFrames),
        m_NTailFrames(nTailFrames),
        m_NChannels(nChannels) {
      assert(nChannels <= MAX_N_CHANNELS);
      assert(nRingFrames > 0);
      // Only the minimum possible margin is checked here - this class does
      // not know the reading resampler's VECTOR_LENGTH (nPoints), so it
      // cannot verify the caller actually left enough room for the mirrored
      // tail the class comment requires. A too-small margin is instead
      // caught where it actually matters, by NextItem()'s p_EndPtr assert.
      assert(nTailFrames > 0);
    }

    /**
     * Makes a one-channel ring over the same memory that reads the given
     * channel of this ring: the same nRingFrames and nTailFrames, the base
     * shifted to the channel's sub-buffer. Cheap (a few words), so planar
     * callers make one per channel and run the interleaved ResampleBlock<...,
     * 1>() on it.
     *
     * @param channelI the channel to take; must be below m_NChannels
     * @return a ring with m_NChannels == 1 reading the given channel. Its read
     *   pointer is unset until Seek() is called, as for a freshly constructed
     *   ring
     */
    inline RingPlanarFrameVector GetChannelVector(unsigned channelI) const {
      assert(channelI < m_NChannels);
      return RingPlanarFrameVector(
        p_RingStart + channelI * (m_NRingFrames + m_NTailFrames),
        1,
        m_NRingFrames,
        m_NTailFrames);
    }

    /**
     * Pins the channel and points to the given index directly - index must
     * already be below nRingFrames (see the class comment). channel must
     * be below m_NChannels (see m_NChannels).
     */
    GO_FORCE_INLINE void Seek(unsigned index, uint_fast8_t channel) {
      assert(channel < m_NChannels);
      assert(index < m_NRingFrames);

      const SrcItemT *pChannelBase
        = p_RingStart + channel * (m_NRingFrames + m_NTailFrames);

      p_CurrPtr = pChannelBase + index;
#ifndef NDEBUG
      p_EndPtr = pChannelBase + m_NRingFrames + m_NTailFrames;
#endif
    }

    /**
     * Returns the seeked channel's next item and advances by one frame.
     * Relies on the ring owner having mirrored each channel's first
     * nTailFrames frames into that channel's own tail, so a run of calls
     * that crosses nRingFrames still reads valid data instead of needing a
     * second wrap; asserts against p_EndPtr so an over-long run (more
     * calls than the mirror covers) fails loudly in Debug instead of
     * silently reading adjacent heap memory.
     */
    GO_FORCE_INLINE ResItemT NextItem() {
      assert(p_CurrPtr < p_EndPtr);

      return (ResItemT) * (p_CurrPtr++);
    }

    /**
     * Keeps resamplingPos's index below this ring's own nRingFrames,
     * called once per output frame by ResampleBlock()/
     * ResampleBlockVariableRatePlanar() (see PtrFrameVector::
     * NormalizePosition() for why this call site is
     * unconditional and zero-cost for non-ring vectors). A single
     * conditional subtract suffices as long as one frame's growth stays
     * below nRingFrames, which holds for any physically sane rate/ring-size
     * pairing. ResampleBlockVariableRatePlanar() checks that precondition
     * up front for every increment (AssertIncrementFits()); the assert
     * below is the per-frame backstop for ResampleBlock().
     */
    inline void NormalizePosition(ResamplingPosition &pos) const {
      const unsigned index = pos.GetIndex();

      if (index >= m_NRingFrames)
        pos.SetIndex(index - m_NRingFrames);
      assert(pos.GetIndex() < m_NRingFrames);
    }

    /**
     * Debug-only check that one resampling step of the given increment
     * (in 1/UPSAMPLE_FACTOR units) cannot advance the index by a whole
     * ring or more, which NormalizePosition()'s single subtract could not
     * correct. Compiled out in Release.
     */
    inline void AssertIncrementFits(unsigned fractionIncrement) const {
      // A step advances the index by at most ceil(increment / UPSAMPLE_FACTOR)
      // frames (the largest starting fraction, UPSAMPLE_MASK, carries one more
      // frame only when the increment has a fractional part); from an index
      // below nRingFrames, a single subtract corrects it iff that is <=
      // nRingFrames
      assert(
        getIndexIncrementByUnits(UPSAMPLE_MASK, fractionIncrement)
        <= m_NRingFrames);
      (void)fractionIncrement;
    }
  };

  // These cofficients are calculated in the constructor and are not more
  // changed

  // The coefficients for linear interpolation
  float m_LinearCoefs[UPSAMPLE_FACTOR][LINEAR_POINTS];
  // The coefficients for polyphase interpolation
  float m_PolyphaseCoefs[UPSAMPLE_FACTOR][POLYPHASE_POINTS];

  GOSoundResample();

  /**
   * A resampler that calculates a next output sample as a scalar production of
   * the vector of continous input samples and the vector of coefficients
   */
  template <unsigned nPoints> class ScalarProductionResampler {
  private:
    // precalculated coefficients for each resampling position fraction
    const float (&r_coefs)[UPSAMPLE_FACTOR][nPoints];

  protected:
    inline ScalarProductionResampler(
      const float (&coefs)[UPSAMPLE_FACTOR][nPoints])
      : r_coefs(coefs) {}

  public:
    /**
     * @return Necessary number of continous input samples for calculating one
     * output sample
     */
    static constexpr unsigned VECTOR_LENGTH = nPoints;

  private:
    /**
     * Computes one output item: the scalar product of nPoints input
     * samples (fV, seeked to pos's current index and the given channel)
     * and the coefficient row for pos's current fraction. Does not advance
     * pos - shared by ResampleBlock() (interleaved output, called once per
     * in-range channel with the same shared resamplingPos) and
     * ResampleBlockVariableRatePlanar() (planar output, called once per
     * channel against that channel's own local position copy) - the only
     * difference between those two callers is what they do with the
     * returned item and how/when they advance the position afterwards, not
     * how the item itself is computed.
     * @param pos the position to seek fV to; not advanced by this call
     * @param fV a floating sample vector linked to the input stream
     * @param ch channel to seek fV to; must be below fV's own channel count
     * @return the computed output item
     */
    template <class FrameVectorT>
    GO_FORCE_INLINE float ComputeOutputItem(
      const ResamplingPosition &pos, FrameVectorT &fV, unsigned ch) const {
      const float(&coefs)[nPoints] = r_coefs[pos.GetFraction()];
      const float *pCoef = coefs;
      float outItem = 0.0f;

      fV.Seek(pos.GetIndex(), ch);
      for (unsigned j = 0; j < nPoints; j++)
        outItem += fV.NextItem() * *(pCoef++);
      return outItem;
    }

  public:
    /**
     * Do actual resampling of an input sample block to the output block of the
     *   given number of frames, at the stream's constant resampling rate
     * @param resamplingPos A resampling position in the input stream. It is
     *   advanced during this call
     * @param fV a floating sample vector linked to the input stream
     * @param pOut a pointer to the output sample buffer in interleaving format.
     *   Must have at least nOutChannels*nOutFrames length
     * @param nOutFrames a number of output frames
     */
    template <class FrameVectorT, uint8_t nOutChannels>
    inline void ResampleBlock(
      ResamplingPosition &resamplingPos,
      FrameVectorT &fV,
      float *pOut,
      unsigned nOutFrames) const {
      // Validated once per block: Inc() of the frame loop has no checks
      assert(resamplingPos.IsValid());
      for (unsigned nFramesLeft = nOutFrames; nFramesLeft > 0; nFramesLeft--) {
        float outItem = 0.0f;

        for (uint8_t ch = 0; ch < nOutChannels; ch++) {
          if (ch < fV.m_NChannels) {
            outItem = ComputeOutputItem(resamplingPos, fV, ch);
          }
          /* else copy the calculated item from the previous channel. It is
           * useful only for resampling a mono stream to a stereo one */
          *(pOut++) = outItem;
        }

        resamplingPos.Inc();
        fV.NormalizePosition(resamplingPos);
      }
    }

    /**
     * Like ResampleBlockVariableRatePlanar(), but the source-position advance
     * is constant: resamplingPos's own m_FractionIncrement is used for every
     * output frame. Writes planar (channel-major) output. Every channel
     * follows the same index/fraction trajectory, so each channel is
     * resampled from a local copy of resamplingPos by the interleaved
     * ResampleBlock() on a one-channel section of fV (fV.GetChannelVector());
     * only the last channel's copy is committed back, so resamplingPos
     * advances exactly nFramesToFill times.
     *
     * @tparam FrameVectorT a planar frame vector type that provides
     *   GetChannelVector(), e.g. RingPlanarFrameVector; other types do not
     *   compile
     * @param resamplingPos a resampling position in the input stream with the
     *   constant increment set; advanced during this call
     * @param fV a floating sample vector linked to the input stream
     * @param nFramesTotalInBuffer each channel's total length, in items, of the
     *   buffer pFirstItemOfChannel0ToFill points into: the stride between one
     *   channel's region and the next
     * @param nChannels number of channels to resample, > 0; must not exceed
     *   fV's own channel count
     * @param pFirstItemOfChannel0ToFill pointer to channel 0's item at the
     *   position this call should start filling
     * @param nFramesToFill how many frames this call writes into each channel;
     *   may be less than nFramesTotalInBuffer
     */
    template <class FrameVectorT>
    inline void ResampleBlockConstantRatePlanar(
      ResamplingPosition &resamplingPos,
      FrameVectorT &fV,
      unsigned nFramesTotalInBuffer,
      unsigned nChannels,
      float *pFirstItemOfChannel0ToFill,
      unsigned nFramesToFill) const {
      assert(nChannels > 0);

      unsigned channelI = 0;
      float *pOut = pFirstItemOfChannel0ToFill;

      do {
        FrameVectorT channelFV = fV.GetChannelVector(channelI);
        ResamplingPosition pos = resamplingPos;

        ResampleBlock<FrameVectorT, 1>(pos, channelFV, pOut, nFramesToFill);
        if (++channelI >= nChannels) {
          resamplingPos = pos; // only the last channel commits back
          break;
        }
        pOut += nFramesTotalInBuffer;
      } while (true);
    }

    /**
     * Like ResampleBlock(), but writes planar (channel-major) output
     * instead of interleaved, and the source-position advance is supplied
     * per output frame instead of being constant, for a time-varying
     * resampling rate (vibrato). resamplingPos's own m_FractionIncrement is
     * never read. This is the only variable-rate entry point in this
     * class (the constant-rate planar one is
     * ResampleBlockConstantRatePlanar()), and this is the method
     * GOSoundVibratoProcessor calls to resample straight into a
     * GOSoundBufferPlanarMutable, with no interleave/de-interleave step and no
     * per-channel-count dispatch.
     *
     * Loop order is channel outer, frame inner - the reverse of
     * ResampleBlock()'s frame-outer/channel-inner order - so each channel
     * is written contiguously into pFirstItemOfChannel0ToFill and read
     * contiguously from a planar FrameVectorT (both channel-major). Every
     * channel follows the identical index/fraction trajectory (only the
     * channel index passed to ComputeOutputItem() differs), so each
     * channel is replayed from a local copy of resamplingPos seeded from
     * the shared starting position, re-walking pPosIncrements from
     * the start; only the last channel's replay commits back into
     * resamplingPos, once, after all channels are done, since a caller
     * must see it advanced exactly nFramesToFill times, not
     * nChannels*nFramesToFill times.
     * @param resamplingPos A resampling position in the input stream. It is
     *   advanced during this call
     * @param fV a floating sample vector linked to the input stream
     * @param pPosIncrements the first of at least nFramesToFill
     *   increments, in 1/UPSAMPLE_FACTOR units (source frames per target
     *   frame, scaled by UPSAMPLE_FACTOR and rounded — see
     *   GOSoundResample::rateToFractionIncrement()). Each is at most
     *   UINT16_MAX, i.e. a rate below 8.0 (UPSAMPLE_FACTOR is 8192): a limit
     *   of the storage, not of the resampler. They must not take
     *   the read position beyond what fV can serve; the limit depends on
     *   FrameVectorT (see its documentation), and Debug builds check what
     *   they can via fV.AssertIncrementFits().
     * @param nFramesTotalInBuffer each channel's total length, in items, of
     *   the buffer pFirstItemOfChannel0ToFill points into (e.g.
     *   GOSoundBufferPlanar::GetNFrames() - the full buffer, not just the
     *   range being filled); this is the stride between one channel's
     *   region and the next, since each channel of a planar buffer holds
     *   one item per frame
     * @param nChannels number of channels to resample; must not exceed fV's
     *   own channel count nor MAX_N_CHANNELS
     * @param pFirstItemOfChannel0ToFill pointer to channel 0's item at the
     *   position this call should start filling - not necessarily the
     *   buffer's own first item, if the caller is filling a sub-range
     * @param nFramesToFill how many frames, starting at
     *   pFirstItemOfChannel0ToFill, this call actually writes - may be less
     *   than nFramesTotalInBuffer if only filling part of the buffer
     */
    template <class FrameVectorT>
    inline void ResampleBlockVariableRatePlanar(
      ResamplingPosition &resamplingPos,
      FrameVectorT &fV,
      const uint16_t *pPosIncrements,
      unsigned nFramesTotalInBuffer,
      unsigned nChannels,
      float *pFirstItemOfChannel0ToFill,
      unsigned nFramesToFill) const {
      assert(nChannels <= MAX_N_CHANNELS);
      assert(resamplingPos.IsValid());
#ifndef NDEBUG
      const uint16_t *pCheckedIncrement = pPosIncrements;

      for (unsigned nFramesLeft = nFramesToFill; nFramesLeft > 0;
           --nFramesLeft, ++pCheckedIncrement)
        fV.AssertIncrementFits(*pCheckedIncrement);
#endif
      for (unsigned ch = 0; ch < nChannels; ch++) {
        ResamplingPosition pos = resamplingPos;
        float *pOut = pFirstItemOfChannel0ToFill + ch * nFramesTotalInBuffer;
        // Every channel replays the increments from the start
        const uint16_t *pIncrement = pPosIncrements;

        for (unsigned nFramesLeft = nFramesToFill; nFramesLeft > 0;
             --nFramesLeft, ++pIncrement) {
          *(pOut++) = ComputeOutputItem(pos, fV, ch);
          pos.Inc(*pIncrement);
          fV.NormalizePosition(pos);
        }
        if (ch + 1 == nChannels)
          resamplingPos = pos;
      }
    }
  };

  /**
   * This resampler use a scalar production of vector of two samples and a
   * vector of two coefficients
   */
  struct LinearResampler : public ScalarProductionResampler<LINEAR_POINTS> {
    inline LinearResampler(const GOSoundResample &r)
      : ScalarProductionResampler<LINEAR_POINTS>(r.m_LinearCoefs) {}
  };

  /**
   * This resampler use a scalar production of vector of eight samples and a
   * vector of eight coefficients
   */
  struct PolyphaseResampler
    : public ScalarProductionResampler<POLYPHASE_POINTS> {
    inline PolyphaseResampler(const GOSoundResample &r)
      : ScalarProductionResampler<POLYPHASE_POINTS>(r.m_PolyphaseCoefs) {}
  };

  /**
   * Returns the necessary sample vector length for the given interpolation type
   * @param interpolation - the interpolation type
   * @return the number of samples
   */
  static unsigned getVectorLength(InterpolationType interpolation);

  /**
   * Allocate a new sample block and fill it with resampled data. Assume that
   *   the samples are mono
   * @param data a pointer to the source samples
   * @param before call - number of source samples. After call it is filled
   *   with number of target samples
   * @param from_samplerate source samplerate
   * @param to_samplerate target samplerate
   * @return a pointer to the new allocated data block. The caller is
   *   responsible to free this block
   */
  float *NewResampledMono(
    const float *data,
    unsigned &len,
    unsigned from_samplerate,
    unsigned to_samplerate);
};

#endif /* GOSOUNDRESAMPLE_H_ */
