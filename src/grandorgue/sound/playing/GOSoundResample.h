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

/**
 * This class provides algorithms for resampling audio buffers
 * Now two algorithms are supported: Linear and Polyphase.
 * They calculate a next output sample based on a vector of a few continous
 * input samples.
 */

class GOSoundResample {
public:
  static constexpr unsigned POLYPHASE_POINTS = 8;
  static constexpr unsigned LINEAR_POINTS = 2;
  static constexpr unsigned UPSAMPLE_BITS = 13;
  static constexpr unsigned UPSAMPLE_FACTOR = 1 << UPSAMPLE_BITS;
  static constexpr unsigned UPSAMPLE_MASK = UPSAMPLE_FACTOR - 1;

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

    /**
     * Calculates the target samples length from given source position to the
     * end index
     * @param endIndex the first index position after the last source sample
     * @result - the number of tatget samples can be received before the current
     *   position exceeds endIndex specified
     */
    inline unsigned AvailableTargetSamples(unsigned endIndex) {
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
   * - void Seek(unsigned index, uint8_t channel) - sets the vector to the index
   *   position in the input stream for the specified channel. The
   *   implementation may restrict moving the position only forward
   * - void NextSample() - returns the next sample of the same channel. The
   *   implementation must provide sufficient number of samples required by
   *   the certain resampling algorithm.
   *
   * These methods are not virtual for a better performance. Their
   * implementation is substituted inline from the subclasses by a resampler.
   */
  template <uint8_t nChannels> struct FloatingSampleVector {
    static constexpr uint8_t m_NChannels = nChannels;
  };

  /**
   * A vector of continous samples in a memory region referenced by a pointer.
   * SampleT - a type of one sample. Usually int8_t, int16_t, GOInt24, or float
   * ResT - a type of one sample returned by NextSample(). Usually int or float
   * nChannels - number of channels in the source stream
   *
   * If nChannels>1 it asumes that samples are interleaving for several channels
   *     - 0 left
   *     - 0 right
   *     - 1 left
   *     - 1 right
   *     - ...
   * For the performance reason this NextSample() does not check for the bounds.
   * The calling program must ensure that there are sufficient number of samples
   */
  template <class SampleT, class ResT, uint8_t nChannels>
  class PtrSampleVector : public FloatingSampleVector<nChannels> {
  private:
    // points to the first sample of the 0-channel in the input stream
    const SampleT *p_StartPtr;
    // points to the current sample in the vector
    const SampleT *p_CurrPtr;

  protected:
    /**
     * Checks that the current sample pointer is before the specified end
     * pointer
     * @param endPtr the end pointer
     * @return are there more samples in the vector
     */
    inline bool IsBefore(const SampleT *endPtr) const {
      return p_CurrPtr < endPtr;
    }

  public:
    /**
     * Construct the vector with some start pointer.
     * @param ptr a pointer to the first (0 left) sample
     */
    inline PtrSampleVector(const SampleT *ptr) : p_StartPtr(ptr) {}

    /**
     * Moves the current sample pointer to the specified position in the input
     * stream for the channel specified
     * @param ptr a pointer to the first (0 left) sample
     */
    inline void Seek(unsigned index, uint8_t channel) {
      p_CurrPtr = p_StartPtr + nChannels * index + channel;
    }

    /**
     * Returns the next sample from the vector and moves the current sample
     * pointer to the next sample of the same channel
     * @return the sample
     */
    inline ResT NextSample() {
      ResT res = (ResT)*p_CurrPtr;
      p_CurrPtr += nChannels;
      return res;
    }
  };

  /**
   * A vector of continous samples in memory with checking for bounds on
   * NextSample().
   * This checking reduces the performance dramatically so it is intended to use
   * only in not realtime cases (for example, on loading, but not when playing)
   */
  template <class SampleT, class ResT, uint8_t nChannels>
  class BoundedPtrSampleVector
    : public PtrSampleVector<SampleT, ResT, nChannels> {
  private:
    /**
     * Points to the end of the memory region
     */
    const SampleT *p_EndPtr;

  public:
    /**
     * Constructs the vector with the start pointer and the length of the
     * memory region
     * @param ptr - a pointer to the first sample in the region
     * @param len - a number of samples of each channels
     */
    inline BoundedPtrSampleVector(const SampleT *ptr, unsigned len)
      : PtrSampleVector<SampleT, ResT, nChannels>(ptr),
        p_EndPtr(ptr + nChannels * len) {}

    inline ResT NextSample() {
      return PtrSampleVector<SampleT, ResT, nChannels>::IsBefore(p_EndPtr)
        ? PtrSampleVector<SampleT, ResT, nChannels>::NextSample()
        : (ResT)0;
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

    /**
     * Do actual resampling of an input sample block to the output block of the
     *   given number of samples
     * @param resamplingPos A resampling position in the input stream. It is
     *   advanced during this call
     * @param sV a floating sample vector linked to the input stream
     * @param pOut a pointer to the output sample buffer in interleaving format.
     *   Must have at least nOutChannels*nOutSamples length
     * @param nOutSamples a number of output samples of each channel
     */
    template <class SampleVectorT, uint8_t nOutChannels>
    inline void ResampleBlock(
      ResamplingPosition &resamplingPos,
      SampleVectorT &sV,
      float *pOut,
      unsigned nOutSamples) const {
      for (unsigned i = 0; i < nOutSamples; i++, resamplingPos.Inc()) {
        const float(&coefs)[nPoints] = r_coefs[resamplingPos.GetFraction()];
        float outSample = 0.0f;

        for (uint8_t ch = 0; ch < nOutChannels; ch++) {
          if (ch < SampleVectorT::m_NChannels) {
            const float *pCoef = coefs;

            sV.Seek(resamplingPos.GetIndex(), ch);
            // calculate the next output sample as a scalar production of the
            // input sample vector and the vector of coefficients
            outSample = 0.0f;
            for (unsigned j = 0; j < nPoints; j++)
              outSample += sV.NextSample() * *(pCoef++);
          }
          /* else copy the calculated sample from the previous channel. It is
           * useful only for resampling a mono stream to a stereo one */
          *(pOut++) = outSample;
        }
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
