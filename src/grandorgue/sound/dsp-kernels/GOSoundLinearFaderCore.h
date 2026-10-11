/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDLINEARFADERCORE_H
#define GOSOUNDLINEARFADERCORE_H

#include <algorithm>
#include <cassert>
#include <cmath>

#include "sound/buffer/GOSoundBufferPlanar.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"

/**
 * Applies a linear gain ramp to interleaved samples given as raw pointers. The
 * template parameter nChannels of fade() and crossfade() is the number of
 * interleaved channels: 2 for an interleaved stereo buffer. A planar buffer
 * is processed channel by channel: crossfadePlanar() does it for the planar
 * buffer classes (the one place where this class knows buffer types), the
 * ramp is shared by all the channels.
 *
 * All the functions are inline or templates, so that the compiler can inline
 * and vectorise them in the callers.
 */
class GOSoundLinearFaderCore {
public:
  /**
   * The gain that silences a signal; in crossfade() the whole weight of
   * pOther (pInOut is replaced).
   */
  static constexpr float GAIN_ZERO = 0.0f;

  /**
   * The gain that leaves a signal unchanged; in crossfade() the whole weight
   * of pInOut (pOther is not heard).
   */
  static constexpr float GAIN_ONE = 1.0f;

  /**
   * Describes one linear ramp. The gain of the frame j counted from the ramp
   * start is startGain + m_DeltaPerFrame * j, 0 <= j < nRampFrames. The fields
   * are private and readable by getters (a built ramp cannot be changed, only
   * assigned as a whole).
   */
  class GainRamp {
  private:
    // the gain of the first frame of the ramp
    float m_StartGain;
    // the gain after the last frame of the ramp; the constant gain of the
    // frames after the ramp end
    float m_EndGain;
    // the gain change per frame; 0 for a constant-gain ramp
    float m_DeltaPerFrame;
    // number of frames of the ramp; 0 for a constant-gain ramp (the gain is
    // m_EndGain everywhere)
    unsigned m_NRampFrames;

  public:
    /**
     * Creates a ramp from startGain to endGain.
     *
     * @param startGain the gain of the first frame (unused if nRampFrames is 0)
     * @param endGain the gain after nRampFrames frames. It is stored: the
     *   constant gain of the frames after the ramp end in fade() and
     *   crossfade()
     * @param nRampFrames number of frames of the ramp. 0 makes a constant-gain
     *   ramp: the gain is endGain everywhere, GetNFramesInRamp() is always 0,
     *   GetGainAt() always returns endGain, the delta is 0. The same ramp is
     *   made when startGain == endGain, whatever nRampFrames is: a ramp that
     *   does not change the gain has no meaningful length, so a ramp with
     *   frames always changes the gain
     */
    inline GainRamp(float startGain, float endGain, unsigned nRampFrames)
      : m_StartGain(
        startGain != endGain && nRampFrames > 0 ? startGain : endGain),
        m_EndGain(endGain),
        m_DeltaPerFrame(
          startGain != endGain && nRampFrames > 0
            ? (endGain - startGain) / nRampFrames
            : 0.0f),
        m_NRampFrames(startGain != endGain ? nRampFrames : 0) {}

    /**
     * Creates the constant-gain ramp with the gain 1: the same as
     * GainRamp(GAIN_ONE, GAIN_ONE, 0). Useful as a member that is assigned
     * later, e.g. in EnsureSetup of a processor.
     */
    inline GainRamp() : GainRamp(GAIN_ONE, GAIN_ONE, 0) {}

    /** @return the gain of the first frame of the ramp */
    inline float GetStartGain() const { return m_StartGain; }

    /**
     * @return the gain after the last frame of the ramp: the constant gain of
     *   the frames after the ramp end
     */
    inline float GetEndGain() const { return m_EndGain; }

    /** @return the gain change per frame; 0 for a constant-gain ramp */
    inline float GetDeltaPerFrame() const { return m_DeltaPerFrame; }

    /**
     * @return the number of frames of the ramp; 0 for a constant-gain ramp
     *   (the gain is the end gain everywhere)
     */
    inline unsigned GetNRampFrames() const { return m_NRampFrames; }

    /**
     * Whether a position is at or past the ramp end.
     *
     * @param frameI the frame position relative to the ramp start
     * @return frameI >= nRampFrames; always true for a constant-gain ramp
     */
    inline bool IsOverAt(unsigned frameI) const {
      return frameI >= m_NRampFrames;
    }

    /**
     * The gain of a frame of the ramp, known to lie inside it. Used by the
     * hot loops; GetGainAt() is the checked variant.
     *
     * @param frameI the frame position relative to the ramp start,
     *   < nRampFrames
     * @return m_StartGain + m_DeltaPerFrame * frameI
     */
    inline float GetGainInside(unsigned frameI) const {
      return m_StartGain + m_DeltaPerFrame * frameI;
    }

    /**
     * The gain of a frame at any position.
     *
     * @param frameI the frame position relative to the ramp start
     * @return m_StartGain + m_DeltaPerFrame * frameI inside the ramp,
     *   endGain from nRampFrames on (so always endGain for a constant-gain
     *   ramp)
     */
    inline float GetGainAt(unsigned frameI) const {
      return IsOverAt(frameI) ? m_EndGain : GetGainInside(frameI);
    }

    /**
     * The sum of the gains of the frames 0 .. frameI - 1, i.e.
     * frameI * startGain + delta * frameI * (frameI - 1) / 2: the distance a
     * cursor moves while its speed follows the ramp. A closed form, not a
     * table. Not rounded: a caller that needs whole frames rounds itself.
     *
     * @param frameI the frame position relative to the ramp start, any value.
     *   Past the ramp end the end gain is summed (a constant-gain ramp gives
     *   frameI * endGain)
     * @return the sum, in frames
     */
    inline double GetGainSumBefore(unsigned frameI) const {
      const double nInside = std::min(frameI, m_NRampFrames);
      const double nOutside = frameI - nInside;

      return (double)m_StartGain * nInside
        + (double)m_DeltaPerFrame * nInside * (nInside - 1.0) / 2.0
        + nOutside * (double)m_EndGain;
    }

    /**
     * How many frames of a piece lie inside the ramp.
     *
     * @param dataOffsetInRamp the offset, in frames, of the piece's data from
     *   the ramp start: the position in the ramp of the piece's first frame
     *   (NOT an offset into the piece's data); any value
     * @param nFrames number of frames in the piece
     * @return the number of the first frames of the piece that are inside the
     *   ramp (0 if the piece starts at or after the ramp end)
     */
    inline unsigned GetNFramesInRamp(
      unsigned dataOffsetInRamp, unsigned nFrames) const {
      return IsOverAt(dataOffsetInRamp)
        ? 0
        : std::min(nFrames, m_NRampFrames - dataOffsetInRamp);
    }

    /**
     * Whether the ramp passes a gain.
     *
     * @param gain the gain to test
     * @return whether gain lies between the start and the end gain of the ramp
     *   (in either order, inclusive), with a slack of half a frame step on
     *   both sides for the float rounding; for a constant-gain ramp only its
     *   own gain
     */
    inline bool IsGainInRange(float gain) const {
      const float slack = 0.5f * std::fabs(m_DeltaPerFrame);

      return gain >= std::min(m_StartGain, m_EndGain) - slack
        && gain <= std::max(m_StartGain, m_EndGain) + slack;
    }

    /**
     * The inverse of GetGainAt(): the frame at which the ramp has the given
     * gain.
     *
     * @param gain the wanted gain; it must be in the range of the ramp
     *   (IsGainInRange(), asserted); for a constant-gain ramp that is its own
     *   gain
     * @return the frame position (relative to the ramp start) nearest to the
     *   frame where the ramp has the gain, clamped to [0, nRampFrames] (the
     *   clamp only absorbs the float slack of IsGainInRange()); so it may be
     *   nRampFrames, where the ramp is over. For a constant-gain ramp
     *   (nRampFrames == 0) it is always 0. A ramp with frames always changes
     *   the gain (the constructor guarantees it), so it can be inverted
     */
    inline unsigned GetGainOffset(float gain) const {
      assert(m_NRampFrames == 0 || m_DeltaPerFrame != 0.0f);
      assert(IsGainInRange(gain));

      return m_NRampFrames > 0 ? (unsigned)std::clamp(
               (gain - m_StartGain) / m_DeltaPerFrame + 0.5f,
               0.0f,
               (float)m_NRampFrames)
                               : 0;
    }
  };

  /**
   * A ramp between the gains 0 and 1 in either direction: the depth of an
   * effect that is switched on or off, or the weight of a crossfade. IsRising()
   * tells where it goes, also after it is over or if it has no frames.
   */
  class UnitRamp : public GainRamp {
  public:
    /**
     * Creates a ramp between GAIN_ZERO and GAIN_ONE.
     *
     * @param isRising true: from GAIN_ZERO to GAIN_ONE; false: from GAIN_ONE to
     *   GAIN_ZERO
     * @param nRampFrames number of frames of the ramp; 0 makes a constant-gain
     *   ramp with the end gain: GAIN_ONE if rising, GAIN_ZERO if not
     */
    inline UnitRamp(bool isRising, unsigned nRampFrames)
      : GainRamp(
        isRising ? GAIN_ZERO : GAIN_ONE,
        isRising ? GAIN_ONE : GAIN_ZERO,
        nRampFrames) {}

    /**
     * Creates the rising constant-gain ramp (the gain 1): the same as
     * UnitRamp(true, 0) and, like GainRamp(), the constant gain 1. Useful as a
     * member that is assigned later.
     */
    inline UnitRamp() : UnitRamp(true, 0) {}

    /** @return whether the ramp goes to (or holds) GAIN_ONE */
    inline bool IsRising() const { return GetEndGain() == GAIN_ONE; }
  };

  /**
   * The direction of a crossfade() in terms of the signal pOther: pInOut is
   * the source signal and the receiver of the result, pOther is the signal the
   * crossfade goes to.
   */
  enum CrossfadeDirection {
    // the weight of pInOut falls 1 -> 0: pInOut is replaced by pOther
    TO_OTHER,
    // the weight of pInOut grows 0 -> 1: pOther is replaced by pInOut
    FROM_OTHER
  };

  /**
   * The only kind of ramp crossfade() accepts: the weight of pInOut, the same
   * meaning as the gain of fade() (the in-place signal is multiplied by it),
   * goes 1 -> 0 (TO_OTHER) or 0 -> 1 (FROM_OTHER); pOther gets the rest. The
   * constructor cannot build anything else.
   */
  class CrossfadeRamp : public UnitRamp {
  public:
    /**
     * Creates a crossfade ramp.
     *
     * @param direction TO_OTHER: from GAIN_ONE to GAIN_ZERO; FROM_OTHER: from
     *   GAIN_ZERO to GAIN_ONE
     * @param nRampFrames number of frames of the ramp; 0 makes a constant-gain
     *   ramp: GAIN_ZERO (take pOther) for TO_OTHER, GAIN_ONE (keep pInOut)
     *   for FROM_OTHER
     */
    inline CrossfadeRamp(CrossfadeDirection direction, unsigned nRampFrames)
      : UnitRamp(direction == FROM_OTHER, nRampFrames) {}

    /**
     * Creates the TO_OTHER constant-gain ramp (the gain 0: take pOther): the
     * same as CrossfadeRamp(TO_OTHER, 0). Useful as a member that is assigned
     * later.
     */
    inline CrossfadeRamp() : CrossfadeRamp(TO_OTHER, 0) {}
  };

  /**
   * Multiplies a piece of the data by the ramp. The gain of every frame inside
   * the ramp is computed by the formula m_StartGain + m_DeltaPerFrame * frameI
   * (not accumulated): the iterations do not depend on each other through the
   * gain, which the compiler vectorises better in the Release build (measured
   * 1.4..4 times faster than the accumulating loop), and pieces of one ramp
   * give the same result as one call bit-for-bit. The frames after the ramp end
   * get the constant gain of the ramp end, dispatched once per call:
   * GAIN_ZERO fills with zeros (also drops NaN and denormals of the input),
   * GAIN_ONE does nothing, any other gain multiplies.
   *
   * @tparam nChannels number of interleaved channels (1 for one channel of a
   *   planar buffer)
   * @param ramp the ramp
   * @param dataOffsetInRamp the offset, in frames, of the data pData points to
   *   from the ramp start: the position in the ramp of its first frame (NOT
   *   an offset into pData); any value (data at or past the ramp end gets the
   *   constant gain of the ramp end)
   * @param nFrames number of frames in the piece. The piece may extend past
   *   the ramp end: the frames after it get the constant gain of the ramp end
   * @param pData interleaved samples, nFrames * nChannels floats, in place
   */
  template <unsigned nChannels>
  static inline void fade(
    const GainRamp &ramp,
    unsigned dataOffsetInRamp,
    unsigned nFrames,
    float *pData) {
    static_assert(nChannels > 0);

    const unsigned nFramesInside
      = ramp.GetNFramesInRamp(dataOffsetInRamp, nFrames);
    const unsigned endRampFrameI = dataOffsetInRamp + nFramesInside;

    for (unsigned frameI = dataOffsetInRamp; frameI < endRampFrameI; ++frameI) {
      const float gain = ramp.GetGainInside(frameI);

      for (unsigned nChannelsLeft = nChannels; nChannelsLeft > 0;
           --nChannelsLeft)
        *pData++ *= gain;
    }
    // The frames after the ramp end: the constant gain of the ramp end
    const float endGain = ramp.GetEndGain();
    const unsigned nItemsLeft = (nFrames - nFramesInside) * nChannels;

    if (endGain == GAIN_ZERO)
      std::fill(pData, pData + nItemsLeft, GAIN_ZERO);
    else if (endGain != GAIN_ONE)
      std::transform(pData, pData + nItemsLeft, pData, [endGain](float v) {
        return v * endGain;
      });
  }

  /**
   * fade() for a planar buffer: every channel is multiplied by fade<1>() with
   * the same ramp and the same position. The frame count is the one of the
   * buffer.
   *
   * @param ramp the ramp
   * @param dataOffsetInRamp the offset, in frames, of the buffer's data from
   *   the ramp start: the position in the ramp of its first frame (NOT an
   *   offset into the buffer); any value; the frames after the ramp end get the
   *   constant gain of the ramp end
   * @param inOutBuffer in: the signal; out: the signal multiplied by the ramp
   */
  static inline void fadePlanar(
    const GainRamp &ramp,
    unsigned dataOffsetInRamp,
    GOSoundBufferPlanarMutable &inOutBuffer) {
    const unsigned nFrames = inOutBuffer.GetNFrames();

    for (unsigned nChannels = inOutBuffer.GetNChannels(), channelI = 0;
         channelI < nChannels;
         ++channelI)
      fade<1>(
        ramp,
        dataOffsetInRamp,
        nFrames,
        inOutBuffer.GetChannelBuffer(channelI).GetData());
  }

  /**
   * Moves a piece of one signal towards another with the ramp:
   * inOut = inOut * g + other * (1 - g), where g is the ramp gain, i.e. the
   * weight of inOut, as in fade(). For TO_OTHER inOut is unchanged at the ramp
   * start and is other at the ramp end; FROM_OTHER is the opposite. Computed as
   * other + (inOut - other) * g (one sub and one fma per sample).
   *
   * @tparam nChannels number of interleaved channels (1 for one channel of a
   *   planar buffer)
   * @param ramp the ramp (the weight of pInOut)
   * @param dataOffsetInRamp the offset, in frames, of the data pOther and
   *   pInOut point to from the ramp start: the position in the ramp of their
   *   first frame (NOT an offset into pOther / pInOut); any value
   * @param nFrames number of frames in the piece. The piece may extend past
   *   the ramp end: the frames after it are pOther (TO_OTHER) or pInOut
   *   (FROM_OTHER), exactly
   * @param pOther the signal the crossfade goes to, nFrames * nChannels
   *   floats; must not overlap pInOut
   * @param pInOut the source signal mixed in place and receiving the result,
   *   nFrames * nChannels floats
   */
  template <unsigned nChannels>
  static inline void crossfade(
    const CrossfadeRamp &ramp,
    unsigned dataOffsetInRamp,
    unsigned nFrames,
    const float *__restrict pOther,
    float *__restrict pInOut) {
    static_assert(nChannels > 0);

    const unsigned nFramesInside
      = ramp.GetNFramesInRamp(dataOffsetInRamp, nFrames);
    const unsigned endRampFrameI = dataOffsetInRamp + nFramesInside;

    for (unsigned frameI = dataOffsetInRamp; frameI < endRampFrameI; ++frameI) {
      const float gain = ramp.GetGainInside(frameI);

      for (unsigned nChannelsLeft = nChannels; nChannelsLeft > 0;
           --nChannelsLeft) {
        *pInOut = *pOther + (*pInOut - *pOther) * gain;
        ++pOther;
        ++pInOut;
      }
    }
    // The frames after the ramp end: the weight of pInOut is the final one of
    // the ramp, GAIN_ONE (keep pInOut: nothing to do) or GAIN_ZERO (take
    // pOther: an exact copy)
    if (ramp.GetEndGain() == GAIN_ZERO)
      std::copy(pOther, pOther + (nFrames - nFramesInside) * nChannels, pInOut);
  }

  /**
   * crossfade() for planar buffers: every channel is mixed by crossfade<1>()
   * from the same ramp and the same position. The frame count is the one of
   * the buffers.
   *
   * @param ramp the ramp (the weight of inOutBuffer)
   * @param dataOffsetInRamp the offset, in frames, of the buffers' data from
   *   the ramp start: the position in the ramp of their first frame (NOT an
   *   offset into the buffers); any value; the frames after the ramp end are
   *   otherBuffer (TO_OTHER) or inOutBuffer (FROM_OTHER), exactly
   * @param otherBuffer the signal the crossfade goes to; the same number of
   *   channels and frames as inOutBuffer; must not overlap it; not changed
   * @param inOutBuffer in: the source signal; out: the result
   */
  static inline void crossfadePlanar(
    const CrossfadeRamp &ramp,
    unsigned dataOffsetInRamp,
    const GOSoundBufferPlanar &otherBuffer,
    GOSoundBufferPlanarMutable &inOutBuffer) {
    const unsigned nFrames = inOutBuffer.GetNFrames();

    assert(otherBuffer.GetNChannels() == inOutBuffer.GetNChannels());
    assert(otherBuffer.GetNFrames() == nFrames);
    for (unsigned nChannels = inOutBuffer.GetNChannels(), channelI = 0;
         channelI < nChannels;
         ++channelI)
      crossfade<1>(
        ramp,
        dataOffsetInRamp,
        nFrames,
        otherBuffer.GetChannelBuffer(channelI).GetData(),
        inOutBuffer.GetChannelBuffer(channelI).GetData());
  }

  /**
   * Whether mapFrameOffset() has a solution. The ramps are independent (their
   * lengths and gain ranges differ), so the gain of a position of fromRamp may
   * have no counterpart in toRamp.
   *
   * The only condition: the current gain of fromRamp lies in the range of
   * toRamp. The position in fromRamp is any: past its end (also for a
   * constant-gain fromRamp) the gain is its end gain. A constant-gain toRamp
   * satisfies it only for its own gain (the result is then 0). For two
   * CrossfadeRamps with frames it always holds (both go through
   * [GAIN_ZERO, GAIN_ONE]); for general GainRamps with different gain ranges
   * it may not.
   *
   * @param fromRamp the ramp being left
   * @param offsetInFromRamp the current position in fromRamp, any value
   * @param toRamp the ramp being entered
   * @return whether the current gain of fromRamp is in the range of toRamp
   */
  static inline bool canMapFrameOffset(
    const GainRamp &fromRamp,
    unsigned offsetInFromRamp,
    const GainRamp &toRamp) {
    return toRamp.IsGainInRange(fromRamp.GetGainAt(offsetInFromRamp));
  }

  /**
   * Finds the position in a ramp that gives the same gain as the current
   * position in another ramp, so that switching from one ramp to the other
   * does not make a gain jump. The ramps may differ in start/end gains and in
   * length.
   *
   * The solution does not exist in every case: canMapFrameOffset() must hold
   * (asserted). There is no clamping and no silent gain jump: a caller that
   * cannot guarantee the conditions checks canMapFrameOffset() and handles
   * the other case itself.
   *
   * @param fromRamp the ramp being left
   * @param offsetInFromRamp the current position in fromRamp
   * @param toRamp the ramp being entered
   * @return the position in toRamp nearest to the one with the same gain, in
   *   [0, number of frames of toRamp]; the number of frames means that toRamp
   *   is over
   */
  static inline unsigned mapFrameOffset(
    const GainRamp &fromRamp,
    unsigned offsetInFromRamp,
    const GainRamp &toRamp) {
    assert(canMapFrameOffset(fromRamp, offsetInFromRamp, toRamp));
    return toRamp.GetGainOffset(fromRamp.GetGainAt(offsetInFromRamp));
  }

  /**
   * Like mapFrameOffset(), but a toRamp without frames is accepted whatever its
   * gain is: the gain jumps to the constant gain of toRamp and the result is 0.
   * For a toRamp with frames it is exactly mapFrameOffset() (its conditions
   * apply). Use it where a zero-length ramp means "instant" by request;
   * mapFrameOffset() stays strict (no silent gain jump), which the fader and
   * the vibrato processor rely on.
   *
   * @param fromRamp the ramp being left
   * @param offsetInFromRamp the current position in fromRamp
   * @param toRamp the ramp being entered
   * @return the position in toRamp: 0 if it has no frames, otherwise the
   *   position nearest to the one with the same gain
   */
  static inline unsigned mapFrameOffsetOrJump(
    const GainRamp &fromRamp,
    unsigned offsetInFromRamp,
    const GainRamp &toRamp) {
    return toRamp.GetNRampFrames() > 0
      ? mapFrameOffset(fromRamp, offsetInFromRamp, toRamp)
      : 0;
  }
};

#endif /* GOSOUNDLINEARFADERCORE_H */
