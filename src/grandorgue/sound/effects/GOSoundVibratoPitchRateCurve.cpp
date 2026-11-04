/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundVibratoPitchRateCurve.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <numeric>

#include <wx/intl.h>
#include <wx/log.h>

/* The rate bounds as int. The public constants are unsigned, like the rates
 * themselves, but normalisePitchRateCurve() works in signed arithmetic: its
 * means, shifts and room to the bounds can be negative, and mixing them with
 * the unsigned constants would silently turn a negative int into a huge
 * unsigned. The static_assert makes the casts below safe. */
static_assert(
  GOSoundVibratoPitchRateCurve::MAX_RATE_UNITS
  <= (unsigned)std::numeric_limits<int>::max());

static constexpr int MIN_RATE_UNITS_INT
  = (int)GOSoundVibratoPitchRateCurve::MIN_RATE_UNITS;
static constexpr int MAX_RATE_UNITS_INT
  = (int)GOSoundVibratoPitchRateCurve::MAX_RATE_UNITS;
static constexpr int MIN_ALLOWED_RATE_UNITS_INT
  = (int)GOSoundVibratoPitchRateCurve::MIN_ALLOWED_RATE_UNITS;
static constexpr int MAX_ALLOWED_RATE_UNITS_INT
  = (int)GOSoundVibratoPitchRateCurve::MAX_ALLOWED_RATE_UNITS;

static_assert(
  std::numeric_limits<float>::is_iec559, "The bit tests below need IEEE 754");

// The IEEE 754 binary32 exponent field: all ones for NaN and infinity
static constexpr uint32_t FLOAT_EXPONENT_MASK = 0x7f800000u;
// The mantissa field: non-zero for NaN, zero for infinity
static constexpr uint32_t FLOAT_MANTISSA_MASK
  = (1u << (std::numeric_limits<float>::digits - 1)) - 1;

/**
 * Whether the float is neither NaN nor infinity, tested on its bits: the
 * release build is compiled with -ffast-math (-ffinite-math-only), where
 * std::isfinite() and the comparisons with a NaN are not reliable.
 */
static bool is_finite(float value) {
  return (std::bit_cast<uint32_t>(value) & FLOAT_EXPONENT_MASK)
    != FLOAT_EXPONENT_MASK;
}

/**
 * Whether the float is NaN (not infinity), tested on its bits - see
 * is_finite().
 */
static bool is_nan(float value) {
  return !is_finite(value)
    && (std::bit_cast<uint32_t>(value) & FLOAT_MANTISSA_MASK);
}

int64_t GOSoundVibratoPitchRateCurve::applyAffineTransform(
  uint16_t *pRates, unsigned nRates, const AffineCoefficients &coefficients) {
  const int64_t kNum = coefficients.kNum;
  const int64_t kDen = coefficients.kDen;

  assert(kDen > 0);

  int64_t newTotalUnits = 0;
  // Half of the divisor is added to round to nearest by the integer division
  const int64_t cNumToRound = coefficients.cNum + kDen / 2;
  uint16_t *pRate = pRates;

  for (unsigned nRatesLeft = nRates; nRatesLeft > 0; --nRatesLeft, ++pRate) {
    /* The int64_t cast: kNum * unsigned would be computed in unsigned. The
     * result is clamped to [MIN_RATE_UNITS, MAX_RATE_UNITS], which fits
     * uint16_t (see the static_assert in the class) */
    const int64_t newRate = std::clamp<int64_t>(
      (kNum * (int64_t)*pRate + cNumToRound) / kDen,
      MIN_RATE_UNITS,
      MAX_RATE_UNITS);

    *pRate = (uint16_t)newRate;
    newTotalUnits += newRate;
  }
  return newTotalUnits;
}

void GOSoundVibratoPitchRateCurve::normalisePitchRateCurve(
  uint16_t *pUnitCurve,
  unsigned curveLen,
  uint64_t currentTotalUnits,
  uint64_t targetTotalUnits) {
  assert(curveLen > 0);
  assert(
    currentTotalUnits < ((uint64_t)1 << 63)
    && targetTotalUnits < ((uint64_t)1 << 63));
  assert(
    std::accumulate(pUnitCurve, pUnitCurve + curveLen, (uint64_t)0)
    == currentTotalUnits);

  // Floor of the target mean; the rounding remainder is spread at the end
  const int targetMean = (int)(targetTotalUnits / curveLen);

  /* The target mean must be at least RATE_MARGIN_UNITS away from both bounds:
   * else the remainder step could push a cell out of the range even when all
   * the deviations are pressed to zero (k == 0). Curves of the processor are
   * normally far from the bounds, so a failure means a pathological curve. */
  assert(targetMean >= MIN_ALLOWED_RATE_UNITS_INT);
  assert(targetMean <= MAX_ALLOWED_RATE_UNITS_INT);

  // Floor of the current mean
  const int currentMean = (int)(currentTotalUnits / curveLen);
  const int meanShift = targetMean - currentMean;
  const auto [pMin, pMax]
    = std::minmax_element(pUnitCurve, pUnitCurve + curveLen);
  const int minCurrentRate = (int)*pMin;
  const int maxCurrentRate = (int)*pMax;

  assert(
    minCurrentRate >= MIN_RATE_UNITS_INT
    && maxCurrentRate <= MAX_RATE_UNITS_INT);

  /* A plain shift of every cell by meanShift is enough if both extreme cells
   * stay within the range narrowed by RATE_MARGIN_UNITS (the remainder is below
   * 1.5 * curveLen, so every cell gets at most 1 (the common part) + 1 (the
   * first cells) more). Otherwise the deviations are pressed. */
  const bool isShiftFit
    = minCurrentRate + meanShift >= MIN_ALLOWED_RATE_UNITS_INT
    && maxCurrentRate + meanShift <= MAX_ALLOWED_RATE_UNITS_INT;

  // What is still to be added to every cell, and to the sum, in the final step
  int pendingShift;
  int64_t nUnitsLeft;

  if (isShiftFit) {
    /* The good and usual case: a plain linear shift of every cell is enough.
     * Nothing is written yet, the shift is done in the final step. */
    pendingShift = meanShift;
    // The remainders of the sums by curveLen: what the whole-frame shift
    // cannot do
    nUnitsLeft = (int64_t)(targetTotalUnits % curveLen)
      - (int64_t)(currentTotalUnits % curveLen);
  } else {
    /* The bad and rare case: a linear shift would take an extreme cell out of
     * the range. First press the curve toward its current mean (scale the
     * deviations from it by k < 1), then shift it onto the target mean. Both
     * are done in one pass here. */

    // How far the deviations from the target mean may go down and up
    const int nUnitsRoomDown = targetMean - MIN_ALLOWED_RATE_UNITS_INT;
    const int nUnitsRoomUp = MAX_ALLOWED_RATE_UNITS_INT - targetMean;
    // The deviations of the extreme cells from the current mean
    const int minCurrentDeviation = minCurrentRate - currentMean;
    const int maxCurrentDeviation = maxCurrentRate - currentMean;

    assert(nUnitsRoomDown >= 0 && nUnitsRoomUp >= 0);
    // An integer minimum cannot exceed the floor of the mean, nor the maximum
    // fall below it
    assert(minCurrentDeviation <= 0 && maxCurrentDeviation >= 0);

    /* The pressing factor k = resCoefficients.kNum / resCoefficients.kDen, as
     * a fraction. It is the smaller of kDownNum / kDownDen (the lowest cell
     * must stay in the range) and kUpNum / kUpDen (the highest cell must stay
     * in the range), i.e. the stronger pressing. The fractions are compared,
     * not divided. A fraction 1/1 means no limit from that side. int64_t: the
     * products do not fit an int. */
    int64_t kDownNum = 1;
    int64_t kDownDen = 1;
    int64_t kUpNum = 1;
    int64_t kUpDen = 1;

    // The lowest cell: targetMean + k * minCurrentDeviation >= MIN_ALLOWED
    if (nUnitsRoomDown < -minCurrentDeviation) {
      kDownNum = nUnitsRoomDown;
      kDownDen = -minCurrentDeviation;
    }
    // The highest cell: targetMean + k * maxCurrentDeviation <= MAX_ALLOWED
    if (nUnitsRoomUp < maxCurrentDeviation) {
      kUpNum = nUnitsRoomUp;
      kUpDen = maxCurrentDeviation;
    }
    /* Compare kDownNum / kDownDen with kUpNum / kUpDen and take the smaller
     * one. Both denominators are positive, so a / b <= c / d is exactly
     * a * d <= c * b, with no division. */
    AffineCoefficients resCoefficients;

    if (kDownNum * kUpDen <= kUpNum * kDownDen) {
      resCoefficients.kNum = kDownNum;
      resCoefficients.kDen = kDownDen;
    } else {
      resCoefficients.kNum = kUpNum;
      resCoefficients.kDen = kUpDen;
    }
    // A plain shift did not fit, so at least one extreme cell limits k
    assert(resCoefficients.kNum < resCoefficients.kDen);

    /* y = targetMean + k * (x - currentMean) = (kNum * x + cNum) / kDen. The
     * extreme cell that limits k lands exactly on a bound of the narrowed
     * range, with no rounding. */
    resCoefficients.cNum
      = resCoefficients.kDen * targetMean - resCoefficients.kNum * currentMean;

    /* Press all the cells toward currentMean and put them around targetMean.
     * The main shift is done by the transform; count in nUnitsLeft what is
     * still missing from the target sum. */
    pendingShift = 0;
    nUnitsLeft = (int64_t)targetTotalUnits
      - applyAffineTransform(pUnitCurve, curveLen, resCoefficients);
  }

  /* The final step: spread the remainder nUnitsLeft (|nUnitsLeft| < 1.5 *
   * curveLen) over the cells in two vectorizable passes. Every cell gets the
   * common part (plus the main shift pendingShift for a plain shift), and
   * the first nExtraCells cells one more unit of the sign of nUnitsLeft. */
  assert(std::abs(nUnitsLeft) / curveLen < 2);

  // nUnitsLeft / curveLen is 0 for a plain shift (|nUnitsLeft| < curveLen)
  const int commonDelta = pendingShift + (int)(nUnitsLeft / curveLen);
  const unsigned nExtraCells = (unsigned)(std::abs(nUnitsLeft) % curveLen);
  const int extraDelta = commonDelta + (nUnitsLeft > 0 ? 1 : -1);

  /* The deltas are small: the shift of the mean plus at most one unit of the
   * remainder */
  assert(std::abs(commonDelta) <= MAX_RATE_UNITS_INT - MIN_RATE_UNITS_INT);
  assert(std::abs(extraDelta) <= MAX_RATE_UNITS_INT - MIN_RATE_UNITS_INT);
  // Only the first nExtraCells cells get one more unit than the others
  assert(std::abs(extraDelta - commonDelta) == 1);
  /* After pressing (pendingShift == 0) the deltas only spread the remainder,
   * |nUnitsLeft| < 2 * curveLen, i.e. at most the margin on either side */
  assert(pendingShift != 0 || std::abs(extraDelta) <= (int)RATE_MARGIN_UNITS);

  uint16_t *pExtraEnd = pUnitCurve + nExtraCells;
  uint16_t *pCurveEnd = pUnitCurve + curveLen;

  // The first nExtraCells cells are increased by extraDelta, the rest by
  // commonDelta
  if (pUnitCurve < pExtraEnd && extraDelta != 0)
    std::transform(pUnitCurve, pExtraEnd, pUnitCurve, [extraDelta](uint16_t u) {
      return (uint16_t)(u + extraDelta);
    });
  if (pExtraEnd < pCurveEnd && commonDelta != 0)
    std::transform(pExtraEnd, pCurveEnd, pExtraEnd, [commonDelta](uint16_t u) {
      return (uint16_t)(u + commonDelta);
    });

#ifndef NDEBUG
  /* Debug only: the result stays in the range (the margin of the plain-shift
   * test, or the pressing, guarantees it). A wrap of the uint16_t narrowing
   * into the range is caught by the sum check below. */
  const auto [pNewMin, pNewMax] = std::minmax_element(pUnitCurve, pCurveEnd);

  assert(*pNewMin >= MIN_RATE_UNITS && *pNewMax <= MAX_RATE_UNITS);
#endif
  assert(
    std::accumulate(pUnitCurve, pUnitCurve + curveLen, (uint64_t)0)
    == targetTotalUnits);
}

void GOSoundVibratoPitchRateCurve::BuildInternal(
  const float *pDeviations,
  unsigned nLoopFrames,
  unsigned nChunkFrames,
  bool isToNormalise) {
  assert(!IsValid());
  assert(nLoopFrames > 0 && nChunkFrames > 0);
  assert((uint64_t)nChunkFrames * MAX_RATE_UNITS < ((uint64_t)1 << 32));

  Resize(nLoopFrames, nChunkFrames);

  uint16_t *pLoop = GetLoopData();

  // Step 1: convert the frequency deviations to increments
  bool isClamped = false;
  const float *pDeviation = pDeviations;
  uint16_t *pRate = pLoop;

  for (unsigned frameI = 0; frameI < nLoopFrames;
       ++frameI, ++pDeviation, ++pRate) {
    const float rawDeviation = *pDeviation;
    float ratio;

    // The bit test also catches NaN and infinity in the fast-math build
    if (
      is_finite(rawDeviation) && rawDeviation >= MIN_DEVIATION
      && rawDeviation <= MAX_DEVIATION)
      ratio = 1.0f + rawDeviation;
    else {
      if (!isClamped) {
        isClamped = true;
        wxLogWarning(
          _("The pitch tremulant curve has the frequency deviation %f at "
            "frame %u, which is outside of the range %.2f .. +%.2f. It is "
            "clamped; further such values are not reported."),
          rawDeviation,
          frameI,
          MIN_DEVIATION,
          MAX_DEVIATION);
      }
      /* NaN means "no deviation"; any other out-of-range value, including an
       * infinity, goes to the bound of its sign */
      ratio = is_nan(rawDeviation)
        ? 1.0f
        : (
          std::signbit(rawDeviation) ? MIN_DEVIATION_RATE : MAX_DEVIATION_RATE);
    }
    // At most MAX_DEVIATION_RATE, fits uint16_t
    *pRate = (uint16_t)GOSoundResample::rateToFractionIncrement(ratio);
  }

  // Steps 2-3 only for a normalised curve (always for Build(), not for some
  // tests)
  if (isToNormalise) {
    /* Step 2: the loop must advance the read position by exactly nLoopFrames
     * frames, i.e. the rates must average exactly 1.0: then the read position
     * does not drift against the write position from cycle to cycle (there is
     * no lag feedback that could correct it). The sum of the increments that
     * advance a position with a zero fraction by nLoopFrames frames is exactly
     * that. */
    const uint64_t actualLoopSumUnits
      = std::accumulate(pLoop, pLoop + nLoopFrames, (uint64_t)0);
    const uint64_t targetLoopSumUnits
      = GOSoundResample::computeMinNUnitsToReach(0, nLoopFrames);

    // Step 3: make the loop sum exactly the target, so the rates average 1.0
    normalisePitchRateCurve(
      pLoop, nLoopFrames, actualLoopSumUnits, targetLoopSumUnits);
  }

  /* Step 4: the tail repeats the start of the loop, so that any chunk of
   * nChunkFrames is contiguous. After the last write to the loop. */
  FillTail();

  /* Step 5: the sum of the chunk starting at every loop index, with a sliding
   * window; the window reads the tail, so this comes after FillTail() */
  o_ChunkSumUnits = std::make_unique_for_overwrite<unsigned[]>(nLoopFrames);

  unsigned *pChunkSumUnits = o_ChunkSumUnits.get();
  // The sliding chunk is [pChunkBegin, pChunkEnd); it starts as the first chunk
  const uint16_t *pChunkBegin = pLoop;
  const uint16_t *pChunkEnd = pLoop + nChunkFrames;
  // The sum of the sliding chunk; initialised with the first chunk
  unsigned currChunkSumUnits
    = std::accumulate(pChunkBegin, pChunkEnd, (unsigned)0);
  unsigned nChunksLeft = nLoopFrames;

  assert(nChunksLeft > 0);
  do {
    *pChunkSumUnits++ = currChunkSumUnits;
    // The last chunk ends at the end of the tail, there is nothing to slide to
    if (--nChunksLeft == 0)
      break;
    // Slide the chunk by one rate; the unsigned wraparound of the difference
    // cancels out
    currChunkSumUnits += *pChunkEnd++ - *pChunkBegin++;
  } while (true);
}

void GOSoundVibratoPitchRateCurve::Build(
  const float *pDeviations, unsigned nLoopFrames, unsigned nChunkFrames) {
  BuildInternal(pDeviations, nLoopFrames, nChunkFrames, true);
}

void GOSoundVibratoPitchRateCurve::Destroy() {
  assert(IsValid());
  o_ChunkSumUnits.reset();
  Resize(0, 0);
}
