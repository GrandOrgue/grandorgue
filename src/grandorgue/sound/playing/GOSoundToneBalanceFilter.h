/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTONEBALANCEFILTER_H_
#define GOSOUNDTONEBALANCEFILTER_H_

#include "sound/GOSoundOnePoleFilter.h"
#include "sound/buffer/GOSoundBufferMutable.h"

/**
 * Maps a tone-balance knob value (-99..99: negative darkens via a low-pass
 * filter, positive brightens via a high-pass filter, 0 disables filtering)
 * to a GOSoundOnePoleFilter::Coeffs, and provides the interleaved-stereo
 * per-sampler state (State) that applies it. The sole consumer of
 * GOSoundOnePoleFilter::processSample() on the sampler side - see
 * sound/effects/GOSoundShelfFilterProcessor for the windchest-effect-side
 * consumer.
 */
class GOSoundToneBalanceFilter {
private:
  unsigned m_samplerate = 0;
  GOSoundOnePoleFilter::Coeffs m_coeffs;

public:
  /** Per-sampler, interleaved-stereo filter memory bound to one
   * GOSoundToneBalanceFilter's coefficients. */
  class State {
  private:
    /** Carried one-pole filter memory, one float per interleaved-stereo
     * channel. */
    float m_state[2];
    // Points directly at the bound filter's m_coeffs, not at the filter
    // itself - State only ever needs the coefficients, so there is no
    // reason to go through a second indirection (a method call on the
    // outer object) on every IsToApply()/ProcessBuffer() call. Set from
    // Init() using a nested class's access to its enclosing class's
    // private members - same privilege as before, just applied to the
    // field instead of a method.
    const GOSoundOnePoleFilter::Coeffs *p_coeffs;

  public:
    State() { Init(nullptr); }

    /**
     * Binds this state to filter's coefficients and zeroes its carried
     * memory.
     * @param filter Coefficient source this state will read from
     *   ProcessBuffer(); may be null (IsToApply() then always false)
     */
    void Init(const GOSoundToneBalanceFilter *filter);

    /** @return whether ProcessBuffer() would change the buffer - false
     * when unbound or the bound Coeffs is isNoop */
    bool IsToApply() { return p_coeffs && !p_coeffs->isNoop; }

    /** Filters outBuffer in place, one interleaved stereo frame at a time,
     * via GOSoundOnePoleFilter::processSample() per channel, reading
     * *p_coeffs directly. Caller must check IsToApply() first - this does
     * not skip work on its own. */
    inline void ProcessBuffer(GOSoundBufferMutable &outBuffer) {
      const unsigned nChannels = outBuffer.GetNChannels();
      float *pData = outBuffer.GetData();
      unsigned channel = 0;

      for (unsigned n = outBuffer.GetNItems(); n > 0;
           n--, pData++, channel = (channel + 1) % nChannels)
        *pData = GOSoundOnePoleFilter::processSample(
          *p_coeffs, *pData, m_state[channel]);
    }
  };

  /**
   * Maps value to a filter type/frequency and computes m_coeffs via
   * GOSoundOnePoleFilter::computeCoeffs(), using the sample rate from the
   * last SetSamplerate() call.
   * @param value Tone-balance knob value; 0 disables filtering
   */
  void Init(int8_t value);

  /** @return whether this filter's Coeffs would change a buffer, i.e.
   * !m_coeffs.isNoop */
  bool IsToApply() const { return !m_coeffs.isNoop; }

  /** Sets the sample rate used by subsequent Init() calls; resets this
   * instance to the identity filter until Init() is called again. */
  void SetSamplerate(unsigned samplerate) {
    m_samplerate = samplerate;
    m_coeffs = GOSoundOnePoleFilter::Coeffs();
  }
};

#endif /* GOSOUNDTONEBALANCE_H_ */
