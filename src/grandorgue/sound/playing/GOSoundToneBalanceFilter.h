/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTONEBALANCEFILTER_H_
#define GOSOUNDTONEBALANCEFILTER_H_

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/dsp-kernels/GOSoundOnePoleFilter.h"
#include "threading/GOVersioned.h"

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
public:
  /** The coefficients published to the playing samplers */
  using CoeffsVersioned = GOVersioned<GOSoundOnePoleFilter::Coeffs>;

private:
  unsigned m_samplerate = 0;
  /** The last value passed to Init(), kept to recompute the coefficients when
   * the sample rate changes */
  int8_t m_value = 0;
  /** The coefficients for m_value at m_samplerate. They are computed and
   * published by the GUI thread (Init() and SetSamplerate()) and followed by
   * the playing samplers (State::EnsureCoeffs()) without locks. */
  CoeffsVersioned m_coeffs;

public:
  /** Per-sampler, interleaved-stereo filter memory and a snapshot of the
   * published GOSoundToneBalanceFilter coefficients. */
  class State {
  private:
    /** Carried one-pole filter memory, one float per interleaved-stereo
     * channel. Valid only while IsToApply(): it is cleared when the filter
     * becomes active */
    float m_state[2] = {0, 0};
    /** The coefficients used by ProcessBuffer(): a consistent copy of the
     * last published coefficients. Identity until the first EnsureCoeffs()
     * that finds a published set. */
    CoeffsVersioned::Snapshot m_coeffs;

  public:
    /* An all-zero State (as left by GOSoundSamplerPool::GetSampler()) is a
     * valid unbound state: it is inactive and takes over the published
     * coefficients at the next EnsureCoeffs(). A copy of a State keeps
     * following the same provider without a discontinuity */

    /**
     * Makes the coefficients of this state follow coeffsVersioned, so the
     * sampler follows changes made while it is playing. Idempotent: copies the
     * coefficients only if a new set has been published since the last call
     * and is not being published at this moment; if it is, the old copy stays
     * in use until the next call. The carried memory is cleared when the
     * filter becomes active, so that it does not start from stale values. Must
     * be called once for each audio buffer before IsToApply() and
     * ProcessBuffer().
     * @param coeffsVersioned the published coefficients to follow, usually
     *   GOSoundProvider::GetToneFilterCoeffs()
     */
    void EnsureCoeffs(const CoeffsVersioned &coeffsVersioned);

    /**
     * @return whether ProcessBuffer() would change the buffer - false when
     *   the coefficients copied by the last EnsureCoeffs() are not isActive
     */
    bool IsToApply() const { return m_coeffs.Get().isActive; }

    /** @return the coefficients that ProcessBuffer() uses. For tests. */
    const GOSoundOnePoleFilter::Coeffs &GetCoeffs() const {
      return m_coeffs.Get();
    }

    /** Filters outBuffer in place, one interleaved stereo frame at a time,
     * via GOSoundOnePoleFilter::processSample() per channel, using the
     * coefficients copied by the last EnsureCoeffs(). Caller must check
     * IsToApply() first - this does not skip work on its own. */
    inline void ProcessBuffer(GOSoundBufferMutable &outBuffer) {
      const GOSoundOnePoleFilter::Coeffs &coeffs = m_coeffs.Get();
      const unsigned nFrames = outBuffer.GetNFrames();
      const unsigned nChannels = outBuffer.GetNChannels();
      float *pData = outBuffer.GetData();

      for (unsigned nFramesLeft = nFrames; nFramesLeft; nFramesLeft--) {
        float *pState = m_state;

        for (unsigned nChannelsLeft = nChannels; nChannelsLeft;
             pState++, pData++, nChannelsLeft--)
          *pData = GOSoundOnePoleFilter::processSample(coeffs, *pData, *pState);
      }
    }
  };

  /**
   * Maps value to a filter type/frequency, computes the coefficients via
   * GOSoundOnePoleFilter::computeCoeffs() using the sample rate from the
   * last SetSamplerate() call and publishes them. It may be called while
   * samplers that use this filter are playing.
   * @param value Tone-balance knob value; 0 disables filtering
   */
  void Init(int8_t value);

  /** @return the value of the last Init() call; 0 if it was not called */
  int8_t GetValue() const { return m_value; }

  /** @return the published coefficients, to be followed by playing samplers */
  const CoeffsVersioned &GetCoeffs() const { return m_coeffs; }

  /** Sets the sample rate and republishes the coefficients of the last Init()
   * value computed for this sample rate (an unfiltered set until Init() is
   * called with a non-zero value). */
  void SetSamplerate(unsigned samplerate);
};

#endif /* GOSOUNDTONEBALANCE_H_ */
