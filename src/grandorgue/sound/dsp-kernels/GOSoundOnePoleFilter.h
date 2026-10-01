/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDONEPOLEFILTER_H_
#define GOSOUNDONEPOLEFILTER_H_

#include <cstdint>

/**
 * A one-pole (first-order) IIR filter: low-pass, high-pass, low-shelf, or
 * high-shelf. Pure static utility - Type, Coeffs, computeCoeffs(), and
 * processSample() are the only shared building blocks; each consumer
 * (GOSoundToneBalanceFilter for the interleaved-stereo per-sampler case,
 * sound/effects/GOSoundShelfFilterProcessor for the generic-channel
 * windchest-effect case) owns its own Coeffs and its own per-channel state
 * shaped for its own buffer format.
 */
class GOSoundOnePoleFilter {
public:
  enum class Type : uint8_t {
    TYPE_NONE = 0,
    TYPE_LPF,
    TYPE_HPF,
    TYPE_LOW_SHELF,
    TYPE_HIGH_SHELF
  };

  /**
   * Coefficients of one one-pole IIR filter, computed by computeCoeffs().
   * Plain data, no virtuals - shared, read-only, safe to pass by const
   * reference into processSample() from any thread.
   */
  struct Coeffs {
    /** Direct-form-II-transposed feedforward/feedback coefficients. The
     * defaults (b0=1, b1=0, a1=0) are a true identity filter - out = in,
     * state stays exactly 0 forever - not all-zero: an all-zero b0 would
     * make processSample() output silence instead of passthrough, a
     * dangerous default for any caller that doesn't check isNoop first. */
    double b0 = 1, b1 = 0, a1 = 0;

    /** True iff this is the identity filter - set by computeCoeffs() for
     * TYPE_NONE, sampleRate == 0, or (for the shelf types) gain == 0.
     * Callers should check this before calling processSample() to skip
     * work a passthrough filter doesn't need. Stored explicitly rather than
     * re-derived from b0/b1/a1 (e.g. b0 == 1 && b1 == 0 && a1 == 0): for
     * gain == 0 the shelf formulas only reduce to those exact values
     * algebraically - relying on that in floating point would make isNoop
     * depend on FP rounding instead of on the actual input conditions that
     * caused it. */
    bool isNoop = true;
  };

  /**
   * Computes the coefficients of a one-pole filter of the given type. Pure
   * function, no allocation/IO - cheap but not free (a handful of
   * cos/sin/pow calls), so callers should call it only when type,
   * frequency, gain, or sampleRate actually changed, not on every audio
   * round.
   * @param type Filter type; TYPE_NONE yields the identity Coeffs
   * @param frequency Cutoff (LPF/HPF) or shelf corner frequency, in Hz
   * @param gain Shelf gain in dB, ignored for LPF/HPF; 0 for a shelf type
   *   yields the identity Coeffs, same as TYPE_NONE
   * @param sampleRate Audio sample rate in Hz; 0 yields the identity Coeffs
   *   (filter not usable without a valid sample rate)
   * @param outCoeffs Receives the computed coefficients, with isNoop set
   *   accordingly
   */
  static void computeCoeffs(
    Type type,
    double frequency,
    double gain,
    unsigned sampleRate,
    Coeffs &outCoeffs);

  /**
   * Applies one sample of the one-pole recurrence in place. inline, no
   * virtual dispatch - safe to call from a per-sampler hot loop with zero
   * overhead. Does not check c.isNoop itself - callers that want to skip
   * no-op filters must check it before calling.
   * @param c Coefficients to apply
   * @param in Input sample
   * @param ioState Carried filter state (one float per channel); updated in
   *   place
   * @return The filtered output sample
   */
  static inline float processSample(const Coeffs &c, float in, float &ioState) {
    float out = c.b0 * in + ioState;

    ioState = c.b1 * in - c.a1 * out;

    return out;
  }
};

#endif /* GOSOUNDONEPOLEFILTER_H_ */
