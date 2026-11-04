/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDVIBRATOPITCHRATECURVE_H
#define GOSOUNDVIBRATOPITCHRATECURVE_H

#include <cassert>
#include <cstdint>
#include <memory>

#include "sound/buffer/GOSoundTypedBufferLoopMonoManaged.h"
#include "sound/dsp-kernels/GOSoundResample.h"

/**
 * A precomputed loop of resampling rates in the units of GOSoundResample,
 * used for the pitch vibrato. Each value is the increment of the resampler's
 * position per one output audio frame, as uint16_t, so a whole chunk can be
 * passed straight to GOSoundResample::ResampleBlockVariableRatePlanar().
 *
 * The loop is normalised: the sum of the increments over the loop is exactly
 * the advance of the resampler's position by as many whole frames as the loop
 * has, as if the rate were constant. Hence the read position of the delay
 * buffer does not drift against the write position from one pass of the loop
 * to the next, however long the vibrato runs. The input of Build() is a
 * relative frequency deviation df/f per frame: the frequency ratio is 1 +
 * deviation. The mean of the input does not matter: the normalisation makes
 * the mean rate exactly 1, so the mean pitch is not shifted.
 *
 * There is no head and no release: the start and the stop of the vibrato are
 * a depth ramp in the tremulant task. The cursor (a LoopPosition) is not here
 * either: it belongs to the tremulant task, which gives the same chunk to all
 * the windchests.
 *
 * Built once by Build() on the control thread (allocates, heavy math), then
 * read-only: any number of threads may call GetChunkDescription() at once.
 * Build() also precomputes the sum of every possible chunk, so the audio thread
 * need not sum the increments of a chunk. Move-only.
 */
class GOSoundVibratoPitchRateCurve
  : public GOSoundTypedBufferLoopMonoManaged<uint16_t> {
public:
  /**
   * The lowest frequency ratio (1 + deviation) a curve accepts: two octaves
   * down. A lower deviation is clamped to it.
   */
  static constexpr float MIN_DEVIATION_RATE = 0.25f;
  /**
   * The highest frequency ratio a curve accepts: two octaves up. A higher
   * deviation is clamped to it.
   */
  static constexpr float MAX_DEVIATION_RATE = 4.0f;
  /** The lowest relative frequency deviation df/f a curve accepts: -0.75. */
  static constexpr float MIN_DEVIATION = MIN_DEVIATION_RATE - 1.0f;
  /** The highest relative frequency deviation df/f a curve accepts: +3. */
  static constexpr float MAX_DEVIATION = MAX_DEVIATION_RATE - 1.0f;

  /**
   * The lowest increment a built curve may hold: 1/5 frame per frame, 1.25
   * times below MIN_DEVIATION_RATE, a headroom for the normalisation.
   * Symmetric to MAX_RATE_UNITS on the logarithmic (pitch) scale.
   */
  static constexpr unsigned MIN_RATE_UNITS
    = GOSoundResample::rateToFractionIncrement(0.2f);
  /**
   * The highest increment a built curve may hold: 5 frames per frame, 1.25
   * times above MAX_DEVIATION_RATE, a headroom for the normalisation. It is a
   * limit of the vibrato, not of the storage; the static_assert below checks
   * that it fits uint16_t.
   */
  static constexpr unsigned MAX_RATE_UNITS
    = GOSoundResample::rateToFractionIncrement(5.0f);
  /**
   * The result range of normalisePitchRateCurve() is narrowed by this on each
   * side: the final remainder step may add up to this many units to a cell.
   */
  static constexpr unsigned RATE_MARGIN_UNITS = 2;
  static constexpr unsigned MIN_ALLOWED_RATE_UNITS
    = MIN_RATE_UNITS + RATE_MARGIN_UNITS;
  static constexpr unsigned MAX_ALLOWED_RATE_UNITS
    = MAX_RATE_UNITS - RATE_MARGIN_UNITS;
  /* the increments, with the margin of the remainder step, are stored as
   * uint16_t */
  static_assert(MAX_RATE_UNITS + RATE_MARGIN_UNITS <= UINT16_MAX);
  /* the increments of the largest allowed deviations fit the allowed range */
  static_assert(
    GOSoundResample::rateToFractionIncrement(MIN_DEVIATION_RATE)
      >= MIN_ALLOWED_RATE_UNITS
    && GOSoundResample::rateToFractionIncrement(MAX_DEVIATION_RATE)
      <= MAX_ALLOWED_RATE_UNITS);

  /**
   * Describes the piece of the loop for one audio period: the increments are
   * not copied, the chunk only points to them. It is nChunkFrames increments
   * long and contiguous in memory, so it can be passed straight to the
   * resampler. Assembled by the tremulant task (a pointer into this curve or
   * into the task's own buffer during a depth ramp), consumed by
   * GOSoundVibratoProcessor::SetPitchRateChunk().
   */
  struct ChunkDescription {
    /** The first increment; nullptr if the vibrato is inactive. */
    const uint16_t *pRateUnits;
    /**
     * The number of increments in the chunk: the curve's nChunkFrames, which
     * must equal the audio period of the consumer; 0 if pRateUnits is nullptr.
     */
    unsigned nFrames;
    /**
     * The sum of the increments of the chunk: the advance of the resampler's
     * position over the chunk; 0 if pRateUnits is nullptr.
     */
    unsigned nTotalUnits;

    /**
     * @return true if the chunk holds increments (pRateUnits is not nullptr)
     */
    inline bool HasRates() const { return pRateUnits; }
  };

private:
  friend class GOTestSoundVibratoPitchRateCurve;     // the unit tests
  friend class GOTestPerfSoundVibratoPitchRateCurve; // the perf tests
  friend class GOTestSoundVibratoProcessor; // the end-to-end tests call
                                            // BuildInternal()

  /**
   * The coefficients of the affine transform y = k * x + c of a rate, with
   * k = kNum / kDen and c = cNum / kDen, so a rate x becomes
   * (kNum * x + cNum) / kDen, rounded to nearest.
   *
   * A plain shift has kNum == kDen == 1 and cNum == the shift of the mean.
   * Pressing has kNum < kDen and cNum == kDen * targetMean - kNum *
   * currentMean. The fields are int64_t because the products of the rates (up
   * to 2^16) with kNum and kDen do not fit an int.
   */
  struct AffineCoefficients {
    int64_t kNum;
    int64_t kDen;
    int64_t cNum;
  };

  /**
   * For every loop index: the sum of the nChunkFrames increments from it (what
   * ChunkDescription::nTotalUnits returns). nLoopFrames items; null when not
   * built. unsigned suffices: nChunkFrames * MAX_RATE_UNITS < 2^32.
   */
  std::unique_ptr<unsigned[]> o_ChunkSumUnits;

  /**
   * Applies the affine transform to nRates rates in place. The results are
   * rounded to nearest (a half rounds up) and clamped to
   * [MIN_RATE_UNITS, MAX_RATE_UNITS], so every result fits uint16_t. Rounding
   * is exact for the results that are not clamped; a negative numerator rounds
   * toward zero, but such a result is below the range anyway and is clamped.
   *
   * It is slower than a plain linear shift: it does an int64_t division per
   * rate and cannot be vectorized. So use it only where a plain shift is not
   * enough, or where the performance does not matter.
   *
   * @param pRates the rates to change in place
   * @param nRates how many rates; may be 0
   * @param coefficients the transform; kDen must be positive
   * @return the sum of the new rates
   */
  static int64_t applyAffineTransform(
    uint16_t *pRates, unsigned nRates, const AffineCoefficients &coefficients);

  /**
   * Does the work of Build(). Differs from it by isToNormalise: if false, the
   * rates are only converted from the deviations (the tail and the chunk sums
   * are still made), but the loop is not normalised to the average 1.0. Such a
   * curve breaks the contract of the processor (a zero-drift loop), so it is
   * only for the tests.
   * @param isToNormalise true for Build()
   * (the other parameters are those of Build())
   */
  void BuildInternal(
    const float *pDeviations,
    unsigned nLoopFrames,
    unsigned nChunkFrames,
    bool isToNormalise);

public:
  /**
   * Changes the rates (in the units of GOSoundResample) in place so that their
   * sum becomes exactly targetTotalUnits and every rate stays within
   * [MIN_RATE_UNITS, MAX_RATE_UNITS]: shifts every rate by the same delta onto
   * the target mean; if that would take an extreme rate out of the range
   * (narrowed by RATE_MARGIN_UNITS), the deviations from the current mean are
   * first pressed toward it by the largest factor k < 1 that keeps the extreme
   * rates in the range. The remainder is spread on the first cells in a
   * vectorizable pass (one unit each), which is what RATE_MARGIN_UNITS is for.
   * @param pUnitCurve curveLen rates to change in place, each within the range
   * @param curveLen the number of rates; must be positive
   * @param currentTotalUnits their sum; both totals must be < 2^63
   * @param targetTotalUnits the new sum; its mean must be at least
   *   RATE_MARGIN_UNITS away from both bounds of the range (the caller must
   *   guarantee this: the margin is what the remainder step spends)
   */
  static void normalisePitchRateCurve(
    uint16_t *pUnitCurve,
    unsigned curveLen,
    uint64_t currentTotalUnits,
    uint64_t targetTotalUnits);

  /**
   * Builds the curve from raw frequency deviations: converts them to
   * increments and normalises the loop so that they average exactly 1.0. The
   * curve must not be built yet (not idempotent): to build it again, call
   * Destroy() first.
   *
   * WARNING: allocates memory and is not cheap. Never call it from an audio
   * thread, and nothing may be reading this instance meanwhile.
   *
   * @param pDeviations nLoopFrames relative frequency deviations df/f, one per
   *   frame; the frequency ratio is 1 + deviation. Read-only, not stored.
   *   Values beyond MIN_DEVIATION .. MAX_DEVIATION are clamped and NaN means no
   *   deviation, with one warning per call
   * @param nLoopFrames the length of the loop; must be positive
   * @param nChunkFrames the length of the chunks it will be read with (the
   *   audio period); must be positive
   */
  void Build(
    const float *pDeviations, unsigned nLoopFrames, unsigned nChunkFrames);

  /**
   * Frees the curve, the opposite of Build(): afterwards IsValid() is false.
   * The curve must be built (not idempotent).
   *
   * WARNING: frees memory. Never call it from an audio thread, and nothing may
   * be reading this instance meanwhile.
   */
  void Destroy();

  /**
   * @param pos the position in the loop; its loop length must equal this
   *   curve's
   * @return the nChunkFrames increments from the position and their sum; valid
   *   until the curve is rebuilt, moved from or destroyed. Called by the
   *   tremulant task on the audio thread in every audio period.
   */
  inline ChunkDescription GetChunkDescription(const LoopPosition &pos) const {
    // The curve must have been built
    assert(o_ChunkSumUnits);
    return {GetChunk(pos), GetNChunkFrames(), o_ChunkSumUnits[pos.GetIndex()]};
  }
};

#endif /* GOSOUNDVIBRATOPITCHRATECURVE_H */
