/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundVibratoPitchRateCurve.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/effects/GOSoundVibratoPitchRateCurve.h"

#include "../GOSoundTestSignals.h"
#include "GOTestScope.h"

const std::string GOTestSoundVibratoPitchRateCurve::TEST_NAME
  = "GOTestSoundVibratoPitchRateCurve";

using Curve = GOSoundVibratoPitchRateCurve;
using LoopPosition = Curve::LoopPosition;

static constexpr unsigned UNIT_RATE = GOSoundResample::UPSAMPLE_FACTOR;

static uint64_t sum_of(const std::vector<uint16_t> &rates) {
  return std::accumulate(rates.begin(), rates.end(), (uint64_t)0);
}

static bool are_all_in_range(const uint16_t *pRates, unsigned nRates) {
  bool res = true;

  for (unsigned rateI = 0; rateI < nRates; ++rateI)
    res = res && pRates[rateI] >= Curve::MIN_RATE_UNITS
      && pRates[rateI] <= Curve::MAX_RATE_UNITS;
  return res;
}

/**
 * @return the first value of the loop of a built curve, followed by its tail;
 *   unlike GetLoopData() it does not invalidate the tail
 */
static const uint16_t *get_rates(const Curve &curve) {
  return curve.GetChunk(LoopPosition(curve.GetNLoopFrames()));
}

static uint16_t random_rate(std::mt19937 &rng) {
  return (
    uint16_t)(Curve::MIN_RATE_UNITS + rng() % (Curve::MAX_RATE_UNITS - Curve::MIN_RATE_UNITS + 1));
}

void GOTestSoundVibratoPitchRateCurve::TestConstants() {
  static_assert(
    Curve::MIN_RATE_UNITS == GOSoundResample::rateToFractionIncrement(0.2f));
  static_assert(
    Curve::MAX_RATE_UNITS == GOSoundResample::rateToFractionIncrement(5.0f));
  static_assert(
    Curve::MIN_ALLOWED_RATE_UNITS
    == Curve::MIN_RATE_UNITS + Curve::RATE_MARGIN_UNITS);
  static_assert(
    Curve::MAX_ALLOWED_RATE_UNITS
    == Curve::MAX_RATE_UNITS - Curve::RATE_MARGIN_UNITS);
  static_assert(Curve::MAX_RATE_UNITS + Curve::RATE_MARGIN_UNITS <= UINT16_MAX);
  // The accepted deviations are the edges of the accepted ratio range
  static_assert(Curve::MIN_DEVIATION == Curve::MIN_DEVIATION_RATE - 1.0f);
  static_assert(Curve::MAX_DEVIATION == Curve::MAX_DEVIATION_RATE - 1.0f);
  static_assert(Curve::MIN_DEVIATION == -0.75f && Curve::MAX_DEVIATION == 3.0f);
  GOAssert(
    Curve::MIN_DEVIATION_RATE == 0.25f && Curve::MAX_DEVIATION_RATE == 4.0f,
    "the rate bounds must be two octaves down and up");

  const Curve curve;

  GOAssert(!curve.IsValid(), "a new curve is empty");
  GOAssert(
    curve.GetNLoopFrames() == 0 && curve.GetNChunkFrames() == 0,
    "the sizes of a new curve are 0");
}

void GOTestSoundVibratoPitchRateCurve::TestApplyAffineTransform() {
  // A plain shift gives x + c exactly
  for (const int64_t shift : {5, -5, 0}) {
    std::vector<uint16_t> rates = {8000, 8192, 9000};

    const int64_t total
      = Curve::applyAffineTransform(rates.data(), rates.size(), {1, 1, shift});

    GOAssert(
      rates[0] == 8000 + shift && rates[1] == 8192 + shift
        && rates[2] == 9000 + shift,
      "a plain shift must add the shift to every rate");
    GOAssert(
      (uint64_t)total == sum_of(rates), "the returned sum must be the new sum");
  }

  // 1000 / 2 = 500 is below the range; 3401 / 2 = 1700.5, a half rounds up
  std::vector<uint16_t> halves = {1000, 3401};

  Curve::applyAffineTransform(halves.data(), halves.size(), {1, 2, 0});
  GOAssert(halves[0] == Curve::MIN_RATE_UNITS, "clamped from below");
  GOAssert(halves[1] == 1701, "a half rounds up");

  // The clamp from above: the result must not wrap uint16_t
  std::vector<uint16_t> big = {60000};

  Curve::applyAffineTransform(big.data(), big.size(), {1, 1, 100000});
  GOAssert(big[0] == Curve::MAX_RATE_UNITS, "clamped from above");

  // A negative numerator is clamped to the minimum
  std::vector<uint16_t> low = {5000};

  Curve::applyAffineTransform(low.data(), low.size(), {1, 1, -100000});
  GOAssert(low[0] == Curve::MIN_RATE_UNITS, "negative is clamped");

  // Compare with an independent computation on random coefficients
  std::mt19937 rng(1);

  for (unsigned iterI = 0; iterI < 500; ++iterI) {
    const int64_t kDen = 1 + rng() % 1000;
    const int64_t kNum = rng() % (kDen + 1);
    const int64_t cNum = (int64_t)(rng() % 2000000) - 1000000;
    std::vector<uint16_t> rates(20);

    for (uint16_t &rate : rates)
      rate = random_rate(rng);

    const std::vector<uint16_t> orig = rates;

    Curve::applyAffineTransform(rates.data(), rates.size(), {kNum, kDen, cNum});
    for (unsigned rateI = 0; rateI < rates.size(); ++rateI) {
      const long double exact
        = ((long double)kNum * orig[rateI] + cNum) / (long double)kDen;
      const long double rounded = std::floor(exact + 0.5L);
      const long double expected = std::clamp<long double>(
        rounded, Curve::MIN_RATE_UNITS, Curve::MAX_RATE_UNITS);

      GOAssert(
        (long double)rates[rateI] == expected,
        "applyAffineTransform must round to nearest and clamp");
    }
  }

  // An empty array
  uint16_t nothing = 77;

  GOAssert(
    Curve::applyAffineTransform(&nothing, 0, {1, 1, 5}) == 0 && nothing == 77,
    "no rates: sum 0 and nothing changed");
}

void GOTestSoundVibratoPitchRateCurve::TestNormaliseRandom() {
  std::mt19937 rng(42);

  for (unsigned iterI = 0; iterI < 3000; ++iterI) {
    const unsigned len = 1 + rng() % 200;
    std::vector<uint16_t> rates(len);

    for (uint16_t &rate : rates)
      rate = random_rate(rng);

    const uint64_t current = sum_of(rates);
    const uint64_t targetMean = Curve::MIN_ALLOWED_RATE_UNITS
      + rng()
        % (Curve::MAX_ALLOWED_RATE_UNITS - Curve::MIN_ALLOWED_RATE_UNITS + 1);
    const uint64_t target = targetMean * len + rng() % len;

    Curve::normalisePitchRateCurve(rates.data(), len, current, target);

    GOAssert(sum_of(rates) == target, "the sum must be exactly the target");
    GOAssert(
      are_all_in_range(rates.data(), len), "every rate must stay in the range");
  }
}

void GOTestSoundVibratoPitchRateCurve::TestNormalisePressed() {
  const unsigned len = 100;
  std::vector<uint16_t> rates(len);

  // A square wave of 2048 / 32768: the mean is much higher than 8192
  for (unsigned rateI = 0; rateI < len; ++rateI)
    rates[rateI] = rateI % 2 ? 32768 : 2048;

  const std::vector<uint16_t> orig = rates;

  Curve::normalisePitchRateCurve(
    rates.data(), len, sum_of(rates), (uint64_t)len * UNIT_RATE);

  GOAssert(sum_of(rates) == (uint64_t)len * UNIT_RATE, "the sum must be exact");
  GOAssert(are_all_in_range(rates.data(), len), "the rates must be in range");
  // The shape is preserved: the low cells stay below the high ones
  for (unsigned rateI = 0; rateI + 1 < len; rateI += 2)
    GOAssert(rates[rateI] < rates[rateI + 1], "the shape is kept");
  // The deviations are shrunk
  GOAssert(rates[1] - rates[0] < orig[1] - orig[0], "the depth must shrink");
}

void GOTestSoundVibratoPitchRateCurve::TestNormaliseExamples() {
  {
    std::vector<uint16_t> rates(20, UNIT_RATE + 1);

    Curve::normalisePitchRateCurve(
      rates.data(), 20, sum_of(rates), (uint64_t)20 * UNIT_RATE);
    GOAssert(
      std::all_of(
        rates.begin(), rates.end(), [](uint16_t r) { return r == UNIT_RATE; }),
      "20 rates of 8193 must become 8192");
  }
  for (const uint16_t constant : {8249, 16384}) {
    std::vector<uint16_t> rates(20, constant);

    Curve::normalisePitchRateCurve(
      rates.data(), 20, sum_of(rates), (uint64_t)20 * UNIT_RATE);
    GOAssert(
      std::all_of(
        rates.begin(), rates.end(), [](uint16_t r) { return r == UNIT_RATE; }),
      "a constant curve must become all 8192");
  }
  {
    std::vector<uint16_t> one = {UNIT_RATE + 1};

    Curve::normalisePitchRateCurve(one.data(), 1, one[0], UNIT_RATE);
    GOAssert(one[0] == UNIT_RATE, "a single rate must hit the target");

    std::vector<uint16_t> two = {8000, 8001};

    Curve::normalisePitchRateCurve(two.data(), 2, 16001, 16385);
    GOAssert(sum_of(two) == 16385, "two rates must hit the target sum");
  }
}

void GOTestSoundVibratoPitchRateCurve::TestBuildLayout() {
  struct LayoutCase {
    unsigned nLoopFrames;
    unsigned nChunkFrames;
  };
  // A long loop; a loop shorter than the tail; one frame per chunk
  const LayoutCase cases[] = {{100, 16}, {5, 16}, {20, 1}};

  for (const LayoutCase &layoutCase : cases) {
    const unsigned nLoop = layoutCase.nLoopFrames;
    const unsigned nChunk = layoutCase.nChunkFrames;
    const std::vector<float> deviations = GOSoundTestSignals::makeSinePeriod(
      nLoop, GOSoundTestSignals::centsToDeviation(100.0f), 37.0f);
    Curve curve;

    curve.Build(deviations.data(), nLoop, nChunk);

    GOAssert(
      curve.GetNLoopFrames() == nLoop && curve.GetNChunkFrames() == nChunk,
      "the sizes must be recorded");

    const uint16_t *pRates = get_rates(curve);

    GOAssert(
      are_all_in_range(pRates, nLoop + nChunk - 1),
      "every rate must be in the range");
    GOAssert(
      std::accumulate(pRates, pRates + nLoop, (uint64_t)0)
        == (uint64_t)nLoop * UNIT_RATE,
      "the loop must average exactly 1.0");
    for (unsigned tailI = 0; tailI + 1 < nChunk; ++tailI)
      GOAssert(
        pRates[nLoop + tailI] == pRates[tailI % nLoop],
        "the tail must repeat the start of the loop (tiled)");
  }

  Curve zero;
  const std::vector<float> zeroDeviations(50, 0.0f);

  zero.Build(zeroDeviations.data(), 50, 8);

  const uint16_t *pZeroRates = get_rates(zero);

  for (unsigned rateI = 0; rateI < 50 + 8 - 1; ++rateI)
    GOAssert(
      pZeroRates[rateI] == UNIT_RATE,
      "zero deviations must give all the unit rates");
}

void GOTestSoundVibratoPitchRateCurve::TestBuildDeepCurve() {
  const unsigned nLoop = 100;
  std::vector<float> deviations(nLoop, Curve::MAX_DEVIATION);

  // The first 30 frames of the loop are at the top, the rest at the bottom
  std::fill(deviations.begin() + 30, deviations.end(), Curve::MIN_DEVIATION);

  Curve curve;

  curve.Build(deviations.data(), nLoop, 16);

  const uint16_t *pRates = get_rates(curve);

  GOAssert(
    are_all_in_range(pRates, nLoop + 15),
    "a deep curve must stay in the range");
  GOAssert(
    std::accumulate(pRates, pRates + nLoop, (uint64_t)0)
      == (uint64_t)nLoop * UNIT_RATE,
    "a deep curve must average exactly 1.0");
  GOAssert(
    pRates[0] > pRates[50],
    "the shape must be preserved: high first, low later");
}

void GOTestSoundVibratoPitchRateCurve::TestBuildClampsDeviations() {
  const float nan = std::nanf("");
  const float inf = INFINITY;
  const std::vector<float> wild = {3.5f, -0.9f, nan, inf, -inf, 0.01f};
  const std::vector<float> clamped = {
    Curve::MAX_DEVIATION,
    Curve::MIN_DEVIATION,
    0.0f,
    Curve::MAX_DEVIATION,
    Curve::MIN_DEVIATION,
    0.01f};
  Curve wildCurve;
  Curve clampedCurve;

  wildCurve.BuildInternal(wild.data(), 6, 4, false);
  clampedCurve.BuildInternal(clamped.data(), 6, 4, false);
  GOAssert(
    std::equal(
      get_rates(wildCurve),
      get_rates(wildCurve) + 6 + 3,
      get_rates(clampedCurve)),
    "out-of-range deviations must be clamped, NaN means no deviation");

  // Both bounds are accepted exactly
  const std::vector<float> edges = {Curve::MAX_DEVIATION, Curve::MIN_DEVIATION};
  Curve edgesCurve;

  edgesCurve.BuildInternal(edges.data(), 2, 4, false);
  GOAssert(
    std::equal(
      get_rates(edgesCurve),
      get_rates(edgesCurve) + 2,
      get_rates(clampedCurve)),
    "the bounds of the deviation range must be accepted exactly");
}

void GOTestSoundVibratoPitchRateCurve::TestBuildInternalNotNormalised() {
  const std::vector<float> deviations(
    10, GOSoundTestSignals::centsToDeviation(1200.0f));
  Curve curve;

  curve.BuildInternal(deviations.data(), 10, 4, false);
  GOAssert(curve.IsValid(), "the curve must be built");

  const uint16_t *pRates = get_rates(curve);

  for (unsigned rateI = 0; rateI < 13; ++rateI)
    GOAssert(
      pRates[rateI] == 2 * UNIT_RATE,
      "+1200 cents without normalisation must stay rate 2");

  const std::vector<float> down(
    10, GOSoundTestSignals::centsToDeviation(-1200.0f));

  curve.Destroy();
  curve.BuildInternal(down.data(), 10, 4, false);

  const uint16_t *pDownRates = get_rates(curve);

  for (unsigned rateI = 0; rateI < 13; ++rateI)
    GOAssert(
      pDownRates[rateI] == UNIT_RATE / 2, "-1200 cents must stay rate 0.5");
}

void GOTestSoundVibratoPitchRateCurve::TestBuildNonZeroMean() {
  const unsigned nLoop = 100;
  std::vector<float> deviations
    = GOSoundTestSignals::makeSinePeriod(nLoop, 0.1f, nLoop);

  // A non-zero mean of the input: the mean ratio would be 1.2
  for (float &deviation : deviations)
    deviation += 0.2f;

  Curve curve;

  curve.Build(deviations.data(), nLoop, 8);

  const uint16_t *pRates = get_rates(curve);

  GOAssert(
    std::accumulate(pRates, pRates + nLoop, (uint64_t)0)
      == (uint64_t)nLoop * UNIT_RATE,
    "a non-zero input mean must be normalised away: the mean rate is 1");
}

void GOTestSoundVibratoPitchRateCurve::TestDestroy() {
  const std::vector<float> deviations(20, 0.0f);
  Curve curve;

  curve.Build(deviations.data(), 20, 4);
  GOAssert(curve.IsValid(), "the curve must be built");
  curve.Destroy();
  GOAssert(!curve.IsValid(), "a destroyed curve is empty");
  GOAssert(
    curve.GetNLoopFrames() == 0 && curve.GetNChunkFrames() == 0,
    "a destroyed curve has zero sizes");
  curve.Build(deviations.data(), 20, 4);
  GOAssert(curve.IsValid(), "a destroyed curve can be built again");
}

void GOTestSoundVibratoPitchRateCurve::TestMoveAndEmpty() {
  const std::vector<float> deviations(20, 0.0f);
  Curve curve;

  curve.Build(deviations.data(), 20, 4);

  Curve moved(std::move(curve));

  GOAssert(!curve.IsValid(), "the source of a move is empty");
  GOAssert(
    moved.GetNLoopFrames() == 20 && moved.GetNChunkFrames() == 4,
    "a moved curve keeps the sizes");

  const LoopPosition pos(20);
  const Curve::ChunkDescription chunk = moved.GetChunkDescription(pos);

  GOAssert(
    chunk.HasRates() && chunk.nFrames == 4
      && chunk.nTotalUnits == 4 * UNIT_RATE,
    "a moved curve still gives its chunks");

  moved = {};
  GOAssert(!moved.IsValid(), "assigning an empty curve empties it");
  moved.Build(deviations.data(), 20, 4);
  GOAssert(moved.IsValid(), "an emptied curve can be rebuilt");
}

void GOTestSoundVibratoPitchRateCurve::TestChunkDescription() {
  struct SumCase {
    unsigned nLoopFrames;
    unsigned nChunkFrames;
    bool isToNormalise;
  };
  // A long loop; a loop shorter than a chunk; one frame per chunk; not
  // normalised
  const SumCase cases[]
    = {{37, 8, true}, {5, 8, true}, {20, 1, true}, {26, 6, false}};

  for (const SumCase &sumCase : cases) {
    const unsigned nLoop = sumCase.nLoopFrames;
    const unsigned nChunk = sumCase.nChunkFrames;
    const std::vector<float> deviations = GOSoundTestSignals::makeSinePeriod(
      nLoop, GOSoundTestSignals::centsToDeviation(300.0f), 11.0f);
    Curve curve;

    curve.BuildInternal(
      deviations.data(), nLoop, nChunk, sumCase.isToNormalise);

    const uint16_t *pRates = get_rates(curve);
    LoopPosition pos(nLoop);

    // Every start index: the cursor goes round the loop several times
    for (unsigned periodI = 0; periodI < 3 * nLoop; ++periodI) {
      const Curve::ChunkDescription chunk = curve.GetChunkDescription(pos);

      GOAssert(
        chunk.pRateUnits == pRates + pos.GetIndex() && chunk.nFrames == nChunk,
        "the chunk must point at the cursor");
      GOAssert(
        chunk.nTotalUnits
          == std::accumulate(chunk.pRateUnits, chunk.pRateUnits + nChunk, 0u),
        "the precomputed sum must equal the direct one");
      pos.Advance(1);
    }
  }
}

void GOTestSoundVibratoPitchRateCurve::TestEmptyChunk() {
  const Curve::ChunkDescription chunk = {nullptr, 0, 0};

  GOAssert(!chunk.HasRates(), "a chunk without rates has no rates");
}

void GOTestSoundVibratoPitchRateCurve::run() {
  GO_RUN_TEST(TestConstants())
  GO_RUN_TEST(TestApplyAffineTransform())
  GO_RUN_TEST(TestNormaliseRandom())
  GO_RUN_TEST(TestNormalisePressed())
  GO_RUN_TEST(TestNormaliseExamples())
  GO_RUN_TEST(TestBuildLayout())
  GO_RUN_TEST(TestBuildDeepCurve())
  GO_RUN_TEST(TestBuildClampsDeviations())
  GO_RUN_TEST(TestBuildInternalNotNormalised())
  GO_RUN_TEST(TestBuildNonZeroMean())
  GO_RUN_TEST(TestDestroy())
  GO_RUN_TEST(TestMoveAndEmpty())
  GO_RUN_TEST(TestChunkDescription())
  GO_RUN_TEST(TestEmptyChunk())
}
