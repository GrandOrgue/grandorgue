/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDVIBRATOPROCESSORSTATE_H
#define GOSOUNDVIBRATOPROCESSORSTATE_H

#include <cassert>

#include "sound/buffer/GOSoundBufferMutableMono.h"
#include "sound/buffer/GOSoundBufferPlanar.h"
#include "sound/buffer/GOSoundBufferPlanarManaged.h"
#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/processing/GOSoundProcessorState.h"

class GOSoundVibratoProcessor;
class GOTestSoundVibratoProcessor;

/**
 * Per-chain-state DSP memory of GOSoundVibratoProcessor. One instance per
 * chain, never shared - GOSoundVibratoProcessor::Process() is const, so
 * every byte of mutable state a chain needs lives here, following the same
 * split as GOSoundShelfFilterProcessorState. The terms "undelayed" and
 * "delayed" signal are defined in the description of GOSoundVibratoProcessor.
 */
class GOSoundVibratoProcessorState : public GOSoundProcessorState {
public:
  /**
   * The phase of the delayed signal in the output, see the class description
   * of GOSoundVibratoProcessor. A phase is changed only by
   * GOSoundVibratoProcessor::CrossfadeWithDelayed() and by the resets.
   */
  enum ActivatingPhase {
    /** No delayed signal in the output; Process() does nothing without
     * rates. */
    INACTIVE,
    /** The activation crossfade: the delayed signal replaces the undelayed
     * one. */
    ACTIVATING,
    /** The delayed signal only. */
    ACTIVE,
    /** The deactivation crossfade: the undelayed signal replaces the delayed
     * one. */
    DEACTIVATING
  };

  /**
   * How many frames the interpolator reads ahead of the read position:
   * GOSoundResample::MAX_POINTS - 1. The read position must always stay this
   * far behind the write position (see GetNFramesAvailableToRead()).
   */
  static constexpr unsigned N_LOOKAHEAD_FRAMES
    = GOSoundResample::MAX_POINTS - 1;
  /**
   * The length of the mirrored tail after the ring: it lets the resampler read
   * across the ring's seam without special-casing the wraparound. It is
   * VECTOR_LENGTH - 1 of the widest resampler: the read index is below the
   * ring's size, so the last tap is that many frames beyond its end.
   */
  static constexpr unsigned N_DELAY_BUFFER_TAIL_FRAMES = N_LOOKAHEAD_FRAMES;

private:
  friend class GOSoundVibratoProcessor;     // Process() reads the ring and the
                                            // positions
  friend class GOTestSoundVibratoProcessor; // direct access for the unit tests

  /**
   * The ring's size in frames, not counting the tail. Stored here because
   * Reset() takes no arguments; set once at construction.
   */
  unsigned m_NDelayBufferRingFrames;
  /**
   * The delay buffer: for each channel the ring of m_NDelayBufferRingFrames
   * frames followed by the mirrored tail of N_DELAY_BUFFER_TAIL_FRAMES frames.
   * See WriteToDelayBuffer() for the invariant of the tail.
   */
  GOSoundBufferPlanarManaged m_DelayBufferRingWithTail;
  /**
   * The index of the next frame to write into the ring, in
   * [0, m_NDelayBufferRingFrames).
   */
  unsigned m_DelayBufferWritePos = 0;
  /**
   * The read position in the ring: the whole-frame index and the fraction.
   * It is moved only by the resampler, never by this class.
   */
  GOSoundResample::ResamplingPosition m_DelayBufferReadResamplingPos;

  /**
   * The phase of the delayed signal; INACTIVE after a reset. The direction of
   * a running crossfade is the phase itself.
   */
  ActivatingPhase m_ActivatingPhase = INACTIVE;

  /**
   * The number of frames already passed in the ramp of the current crossfade
   * (GOSoundVibratoProcessor::m_ActivationRamp in ACTIVATING,
   * m_DeactivationRamp in DEACTIVATING); only grows, except for a recalculation
   * at a change of the direction. Always inside the ramp while crossfading; 0
   * in INACTIVE and ACTIVE, so a state after a crossfade equals a fresh one.
   */
  unsigned m_CrossfadePos = 0;

  /**
   * The number of frames of the ramp that m_CrossfadePos was counted in during
   * the last period of the crossfade; 0 in INACTIVE and ACTIVE. If the
   * processor's ramp has another length now (a setter has changed it), the
   * position is recalculated to the new ramp.
   */
  unsigned m_LastCrossfadeNFrames = 0;

  /**
   * @param channelI the channel index
   * @param startPos the first frame of the sub-buffer in the ring with its tail
   * @param nSubBufferFrames the sub-buffer's length in frames
   * @return the sub-buffer of the given channel of the delay buffer (the ring
   *   followed by the tail)
   */
  inline GOSoundBufferMutableMono GetDelayMonoSubBuffer(
    unsigned channelI, unsigned startPos, unsigned nSubBufferFrames) {
    return m_DelayBufferRingWithTail.GetChannelBuffer(channelI).GetSubBuffer(
      startPos, nSubBufferFrames);
  }

public:
  /**
   * Allocates the ring with its tail and primes the ring as Reset() does.
   * @param nChannels the number of channels this state will be processed with
   * @param nRingFrames the ring's size in frames, not counting the tail
   */
  GOSoundVibratoProcessorState(unsigned nChannels, unsigned nRingFrames);

  /**
   * The free room for the next write: the forward distance from the write
   * position to the read index, in [0, ring - N_LOOKAHEAD_FRAMES]. 0 means a
   * full ring; an empty one is impossible, because the read position always
   * stays at least N_LOOKAHEAD_FRAMES behind the write position.
   */
  inline unsigned GetNFramesAvailableToWrite() const {
    const unsigned writePos = m_DelayBufferWritePos;
    const unsigned readIndex = m_DelayBufferReadResamplingPos.GetIndex();

    assert(writePos < m_NDelayBufferRingFrames);
    assert(readIndex < m_NDelayBufferRingFrames);

    const unsigned nFramesAvailableToWrite = readIndex >= writePos
      ? readIndex - writePos
      : readIndex + m_NDelayBufferRingFrames - writePos;

    assert(
      nFramesAvailableToWrite <= m_NDelayBufferRingFrames - N_LOOKAHEAD_FRAMES);
    return nFramesAvailableToWrite;
  }

  /**
   * By how many whole frames the read position may advance:
   * ring - N_LOOKAHEAD_FRAMES - GetNFramesAvailableToWrite().
   */
  inline unsigned GetNFramesAvailableToRead() const {
    return m_NDelayBufferRingFrames - N_LOOKAHEAD_FRAMES
      - GetNFramesAvailableToWrite();
  }

  /**
   * Appends all frames of srcBuffer to the delay buffer at the write position,
   * wrapping at the ring's end, and keeps the mirrored tail consistent: every
   * ring frame written at an index below N_DELAY_BUFFER_TAIL_FRAMES is also
   * copied to the tail right after the ring, so after the call
   * tail[j] == ring[j] for every index j < N_DELAY_BUFFER_TAIL_FRAMES written
   * since the last reset (the other tail entries are unspecified and never
   * read). Advances the write position by srcBuffer.GetNFrames().
   * The read position is not touched.
   * @param srcBuffer the frames to write; must have as many channels as the
   *   ring, and no more frames than GetNFramesAvailableToWrite()
   */
  void WriteToDelayBuffer(const GOSoundBufferPlanar &srcBuffer);

  /**
   * Zero-fills the first N_LOOKAHEAD_FRAMES frames of the ring of every
   * channel, puts the write position to N_LOOKAHEAD_FRAMES and the read
   * position to the start, makes the phase INACTIVE with the crossfade
   * position 0. Does not touch the rest of the ring and the tail. Required by
   * GOSoundProcessorState: called when the chain's content is discarded, by
   * the constructor, and by Process() when the delayed signal has faded out.
   *
   * The tail is deliberately not zeroed, although it mirrors ring[0,
   * N_DELAY_BUFFER_TAIL_FRAMES): it is read only when the read index is within
   * N_LOOKAHEAD_FRAMES of the ring's end, and the read position always stays
   * at least N_LOOKAHEAD_FRAMES behind the write position. In the first lap
   * after the reset the read index is below ring - N_LOOKAHEAD_FRAMES, so the
   * tail is not touched. Before the read index gets that far, the write
   * position has wrapped, and WriteToDelayBuffer() has overwritten the ring's
   * start together with its mirror in the tail. So stale data in the tail is
   * never read.
   */
  void Reset() override;
};

#endif /* GOSOUNDVIBRATOPROCESSORSTATE_H */
