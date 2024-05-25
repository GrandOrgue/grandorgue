/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
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

#include <cstdint>

struct GOSoundResample {
  static constexpr unsigned POLYPHASE_BITS = 3;
  static constexpr unsigned POLYPHASE_POINTS = 1 << POLYPHASE_BITS;
  static constexpr unsigned LINEAR_POINTS = 2;
  static constexpr unsigned UPSAMPLE_BITS = 13;
  static constexpr unsigned UPSAMPLE_FACTOR = 1 << UPSAMPLE_BITS;
  static constexpr unsigned UPSAMPLE_MASK = UPSAMPLE_FACTOR - 1;

  /* This factor must not be exceeded in the downsampler and it MUST be
   * greater than UPSAMPLE_FACTOR.
   *
   * The value of 2663 was tuned analytically and results in a filter power
   * ripple of less than 0.00002 dB when the sub filter taps is 8 and the
   * upsample factor is 2048.
   *
   * The roll off characteristic starts at:
   *  (UPSAMPLE_FACTOR * sample_rate) / (MAX_POSITIVE_FACTOR * 2) ~ 18kHz at
   * 48 kHz. */
  static constexpr unsigned MAX_POSITIVE_FACTOR = 2663;

  enum InterpolationType {
    GO_LINEAR_INTERPOLATION = 0,
    GO_POLYPHASE_INTERPOLATION = 1,
  };

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

  public:
    inline unsigned GetIndex() const { return m_index; }
    inline void SetIndex(unsigned newIndex) { m_index = newIndex; }
    inline unsigned GetFraction() const { return m_fraction; }
    inline unsigned GetFractionIncrement() const { return m_FractionIncrement; }

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
     * Advance the position for the next target sample
     */
    inline void Inc() {
      m_fraction += m_FractionIncrement;
      m_index += m_fraction >> UPSAMPLE_BITS;
      m_fraction &= UPSAMPLE_MASK;
    }
  };

  template <class SampleT, uint8_t nChannels> class PointerWindow {
  private:
    static constexpr uint8_t m_NChannels = nChannels;

    const SampleT *p_StartPtr;
    const SampleT *p_EndPtr;
    const SampleT *p_CurrPtr;

  public:
    inline PointerWindow(const SampleT *ptr, unsigned to)
      : p_StartPtr(ptr), p_EndPtr(ptr + m_NChannels * to) {}

    inline void Seek(unsigned index, uint8_t channel) {
      p_CurrPtr = p_StartPtr + m_NChannels * index + channel;
    }

    inline float NextSample() {
      float res = 0.0f;

      if (p_CurrPtr < p_EndPtr) {
        res = (float)*p_CurrPtr;
        p_CurrPtr += m_NChannels;
      }
      return res;
    }
  };

  float m_PolyphaseCoefs[UPSAMPLE_FACTOR][POLYPHASE_POINTS];
  float m_LinearCoeffs[UPSAMPLE_FACTOR][LINEAR_POINTS];
  InterpolationType m_interpolation;

  void Init(
    const unsigned input_sample_rate,
    GOSoundResample::InterpolationType interpolation);

  template <class SourceT>
  inline float CalcLinear(unsigned fraction, SourceT &source) const {
    const float(&coef)[LINEAR_POINTS] = m_LinearCoeffs[fraction];

    return source.NextSample() * coef[1] + source.NextSample() * coef[0];
  }

  template <class SourceT>
  inline float CalcPolyphase(unsigned fraction, SourceT &source) const {
    const float *pCoef = m_PolyphaseCoefs[fraction];
    float out = 0.0f;

    for (unsigned j = 0; j < POLYPHASE_POINTS; j++)
      out += source.NextSample() * *(pCoef++);
    return out;
  }

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
  static float *newResampledMono(
    const float *data,
    unsigned &len,
    unsigned from_samplerate,
    unsigned to_samplerate);
};

#endif /* GOSOUNDRESAMPLE_H_ */
