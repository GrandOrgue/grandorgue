/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTONEBALANCEFILTER_H_
#define GOSOUNDTONEBALANCEFILTER_H_

#include <atomic>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/dsp-kernels/GOSoundOnePoleFilter.h"

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
  /** A seqlock counter of m_coeffs. It is odd while m_coeffs is being written
   * by Init() or SetSamplerate() (called from the GUI thread) and is increased
   * to the next even value when the write is finished. The audio thread copies
   * m_coeffs only if the counter was even and was not changed during copying,
   * so it never uses a half-written set of coefficients. */
  std::atomic<unsigned> m_Version{0};

  /** Publishes new coefficients so that State::EnsureCoeffs() never sees a torn
   * mix of the old and new values. */
  void Publish(const GOSoundOnePoleFilter::Coeffs &newCoeffs);

public:
  /** Per-sampler, interleaved-stereo filter memory bound to one
   * GOSoundToneBalanceFilter's coefficients. */
  class State {
  private:
    /** Carried one-pole filter memory, one float per interleaved-stereo
     * channel. */
    float m_state[2];
    // The bound filter. The coefficients are not read from it directly in
    // ProcessBuffer() (they may be changed by another thread at any time):
    // EnsureCoeffs() copies them into m_coeffs between audio buffers
    const GOSoundToneBalanceFilter *p_filter;
    /** The coefficients used by ProcessBuffer(): a consistent copy of the
     * bound filter's coefficients. Identity until the first EnsureCoeffs(). */
    GOSoundOnePoleFilter::Coeffs m_coeffs;
    /** The filter's m_Version that m_coeffs was copied at. Odd means "not
     * copied yet" because a finished write always has an even version. */
    unsigned m_Version;

  public:
    State() { Init(nullptr); }

    /**
     * Binds this state to filter's coefficients and zeroes its carried
     * memory.
     * @param filter Coefficient source this state will read from
     *   ProcessBuffer(); may be null (IsToApply() then always false)
     */
    void Init(const GOSoundToneBalanceFilter *filter);

    /**
     * Makes the coefficients of this state follow the bound filter, so the
     * sampler follows changes made while it is playing. Idempotent: copies the
     * coefficients only if the filter has published a new set since the last
     * call and is not writing one at this moment; if it is writing, the old
     * copy stays in use until the next call. The carried memory is cleared when
     * the filter becomes inactive, so that it does not start from stale values
     * when it becomes active again. Does nothing when unbound. Must be called
     * once for each audio buffer before IsToApply() and ProcessBuffer().
     */
    void EnsureCoeffs();

    /**
     * @return whether ProcessBuffer() would change the buffer - false when
     *   unbound or the coefficients copied by the last EnsureCoeffs() are not
     *   isActive
     */
    bool IsToApply() const { return m_coeffs.isActive; }

    /** @return the coefficients that ProcessBuffer() uses. For tests. */
    const GOSoundOnePoleFilter::Coeffs &GetCoeffs() const { return m_coeffs; }

    /** Filters outBuffer in place, one interleaved stereo frame at a time,
     * via GOSoundOnePoleFilter::processSample() per channel, using the
     * coefficients copied by the last EnsureCoeffs(). Caller must check
     * IsToApply() first - this does not skip work on its own. */
    inline void ProcessBuffer(GOSoundBufferMutable &outBuffer) {
      const unsigned nFrames = outBuffer.GetNFrames();
      const unsigned nChannels = outBuffer.GetNChannels();
      float *pData = outBuffer.GetData();

      for (unsigned nFramesLeft = nFrames; nFramesLeft; nFramesLeft--) {
        float *pState = m_state;

        for (unsigned nChannelsLeft = nChannels; nChannelsLeft;
             pState++, pData++, nChannelsLeft--)
          *pData
            = GOSoundOnePoleFilter::processSample(m_coeffs, *pData, *pState);
      }
    }
  };

  /**
   * Maps value to a filter type/frequency and computes m_coeffs via
   * GOSoundOnePoleFilter::computeCoeffs(), using the sample rate from the
   * last SetSamplerate() call. It may be called while samplers that use this
   * filter are playing.
   * @param value Tone-balance knob value; 0 disables filtering
   */
  void Init(int8_t value);

  /** @return whether this filter's Coeffs would change a buffer, i.e.
   * m_coeffs.isActive */
  bool IsToApply() const { return m_coeffs.isActive; }

  /** Sets the sample rate used by subsequent Init() calls; resets this
   * instance to the identity filter until Init() is called again. */
  void SetSamplerate(unsigned samplerate);
};

#endif /* GOSOUNDTONEBALANCE_H_ */
