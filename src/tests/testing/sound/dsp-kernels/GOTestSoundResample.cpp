/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundResample.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/dsp-kernels/GOSoundResample.h"

const std::string GOTestSoundResample::TEST_NAME = "GOTestSoundResample";

static void fill_with_ramp(std::vector<float> &data) {
  for (unsigned n = data.size(), i = 0; i < n; i++)
    data[i] = (float)i;
}

/**
 * Relative comparison for two outputs computed by the same mathematical
 * formula along different code paths. Not bit-exact ==: under the Release
 * flags (-O3 -ffast-math) each ComputeOutputItem() instantiation may be
 * reassociated independently, so results differ by a few ULPs.
 */
static bool are_close(float expected, float actual) {
  return std::fabs(expected - actual)
    <= 1e-4f * std::max(1.0f, std::fabs(expected));
}

template <class ResamplerT>
static void assert_constant_rate_equivalent(
  GOTestSoundResample &test,
  const GOSoundResample &resampler,
  ResamplerT typedResampler,
  const std::string &caseName) {
  static constexpr unsigned N_CHANNELS = 2;
  static constexpr unsigned N_OUT_FRAMES = 16;
  static constexpr unsigned N_SRC_FRAMES = 64;
  std::vector<float> src(N_CHANNELS * N_SRC_FRAMES);

  // Per-channel offset so a channel-stride bug shows up as a mismatch, not
  // a coincidentally-equal one.
  for (unsigned frameI = 0; frameI < N_SRC_FRAMES; frameI++)
    for (unsigned ch = 0; ch < N_CHANNELS; ch++)
      src[frameI * N_CHANNELS + ch] = (float)(ch * 1000 + frameI);

  GOSoundResample::ResamplingPosition posInterleaved;
  GOSoundResample::ResamplingPosition posPlanar;

  posInterleaved.Init(1.37f);
  posPlanar.Init(1.37f);

  GOSoundResample::PtrFrameVector<float, float, N_CHANNELS> fvInterleaved(
    src.data());
  GOSoundResample::PtrFrameVector<float, float, N_CHANNELS> fvPlanar(
    src.data());
  std::vector<unsigned> increments(
    N_OUT_FRAMES, posPlanar.GetFractionIncrement());

  std::vector<float> outInterleaved(N_CHANNELS * N_OUT_FRAMES);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(planarOut, N_CHANNELS, N_OUT_FRAMES)
  std::vector<float> planarAsInterleaved(N_CHANNELS * N_OUT_FRAMES);
  GOSoundBufferMutable planarAsInterleavedBuf(
    planarAsInterleaved.data(), N_CHANNELS, N_OUT_FRAMES);

  typedResampler.template ResampleBlock<
    GOSoundResample::PtrFrameVector<float, float, N_CHANNELS>,
    N_CHANNELS>(
    posInterleaved, fvInterleaved, outInterleaved.data(), N_OUT_FRAMES);
  typedResampler.template ResampleBlockVariableRatePlanar<
    GOSoundResample::PtrFrameVector<float, float, N_CHANNELS>>(
    posPlanar,
    fvPlanar,
    increments.data(),
    N_OUT_FRAMES,
    N_CHANNELS,
    planarOut.GetData(),
    N_OUT_FRAMES);
  planarOut.InterleaveTo(planarAsInterleavedBuf);

  // Relative tolerance, not bit-exact ==: ResampleBlock() and
  // ResampleBlockVariableRatePlanar() both call the same ComputeOutputItem(),
  // but from different surrounding loop shapes (frame-outer/channel-inner
  // vs. channel-outer/frame-inner) - under this project's Release build
  // flags (-ffast-math is on, confirmed via GOTestPerfSoundResample's own
  // build), the compiler is free to reassociate/contract each call site's
  // floating-point ops independently, so the two paths can differ by a few
  // ULPs even though they compute the same mathematical value from the same
  // inputs. Debug (no -ffast-math) matches bit-exactly.
  for (unsigned itemI = 0, n = N_CHANNELS * N_OUT_FRAMES; itemI < n; itemI++) {
    test.GOAssert(
      are_close(outInterleaved[itemI], planarAsInterleaved[itemI]),
      caseName + ": output item " + std::to_string(itemI)
        + " must match within tolerance");
  }
  test.GOAssert(
    posInterleaved.GetIndex() == posPlanar.GetIndex(),
    caseName + ": final index must match");
  test.GOAssert(
    posInterleaved.GetFraction() == posPlanar.GetFraction(),
    caseName + ": final fraction must match");
}

void GOTestSoundResample::TestConstantRateEquivalence() {
  GOSoundResample resampler;

  assert_constant_rate_equivalent(
    *this, resampler, GOSoundResample::LinearResampler(resampler), "LINEAR");
  assert_constant_rate_equivalent(
    *this,
    resampler,
    GOSoundResample::PolyphaseResampler(resampler),
    "POLYPHASE");
}

void GOTestSoundResample::TestVaryingRateCorrectness() {
  // Hand-picked increments, in 1/UPSAMPLE_FACTOR units.
  static constexpr unsigned UPSAMPLE_FACTOR = GOSoundResample::UPSAMPLE_FACTOR;
  const unsigned increments[] = {
    UPSAMPLE_FACTOR / 2,
    UPSAMPLE_FACTOR,
    UPSAMPLE_FACTOR * 3 / 2,
    UPSAMPLE_FACTOR / 4};
  static constexpr unsigned N_OUT_FRAMES = 4;

  std::vector<float> src(8);

  fill_with_ramp(src);

  // Independently derived trajectory (index, fraction) -> Inc(unsigned)'s
  // documented formula, and LINEAR_INTERPOLATION's coefficients are
  // (1 - fraction/UPSAMPLE_FACTOR, fraction/UPSAMPLE_FACTOR):
  //   S0 = (index=0, fraction=0)
  //   frame0: coefs=(1, 0), reads src[0],src[1] -> 1*0 + 0*1 = 0.0
  //     Inc(4096): fraction=4096, index+=0 -> S1=(0, 4096)
  //   frame1: coefs=(0.5, 0.5), reads src[0],src[1] -> 0.5*0+0.5*1 = 0.5
  //     Inc(8192): fraction=12288&8191=4096, index+=1 -> S2=(1, 4096)
  //   frame2: coefs=(0.5, 0.5), reads src[1],src[2] -> 0.5*1+0.5*2 = 1.5
  //     Inc(12288): fraction=16384&8191=0, index+=2 -> S3=(3, 0)
  //   frame3: coefs=(1, 0), reads src[3],src[4] -> 1*3+0*4 = 3.0
  //     Inc(2048): fraction=2048, index+=0 -> S4=(3, 2048)
  const float expectedOutput[N_OUT_FRAMES] = {0.0f, 0.5f, 1.5f, 3.0f};
  constexpr unsigned expectedFinalIndex = 3;
  constexpr unsigned expectedFinalFraction = 2048;

  GOSoundResample resampler;
  GOSoundResample::LinearResampler linearResampler(resampler);
  GOSoundResample::ResamplingPosition pos;
  std::vector<float> out(N_OUT_FRAMES);

  pos.Init(1.0f);

  GOSoundResample::PtrFrameVector<float, float, 1> fv(src.data());

  // Mono: planar and interleaved layouts coincide, so this writes directly
  // into out.data() with no conversion needed.
  linearResampler.ResampleBlockVariableRatePlanar<
    GOSoundResample::PtrFrameVector<float, float, 1>>(
    pos, fv, increments, N_OUT_FRAMES, 1, out.data(), N_OUT_FRAMES);

  for (unsigned frameI = 0; frameI < N_OUT_FRAMES; frameI++)
    GOAssert(
      out[frameI] == expectedOutput[frameI],
      "frame " + std::to_string(frameI)
        + ": output must match hand-computed value");
  GOAssert(pos.GetIndex() == expectedFinalIndex, "final index must match");
  GOAssert(
    pos.GetFraction() == expectedFinalFraction, "final fraction must match");
}

void GOTestSoundResample::TestRingNonWrappingReads() {
  static constexpr uint8_t N_CHANNELS = 2;
  static constexpr unsigned N_RING_FRAMES = 10;
  static constexpr unsigned N_TAIL_FRAMES = 4;
  static constexpr unsigned N_PHYSICAL_FRAMES = N_RING_FRAMES + N_TAIL_FRAMES;

  std::vector<float> planar(N_CHANNELS * N_PHYSICAL_FRAMES, -1.0f);
  std::vector<float> interleaved(N_CHANNELS * N_RING_FRAMES);

  for (uint8_t ch = 0; ch < N_CHANNELS; ch++)
    for (unsigned frameI = 0; frameI < N_RING_FRAMES; frameI++) {
      const float value = ch * 1000.0f + frameI;

      planar[ch * N_PHYSICAL_FRAMES + frameI] = value;
      interleaved[frameI * N_CHANNELS + ch] = value;
    }

  GOSoundResample::RingPlanarFrameVector<float, float> ring(
    planar.data(), N_CHANNELS, N_RING_FRAMES, N_TAIL_FRAMES);
  GOSoundResample::PtrFrameVector<float, float, N_CHANNELS> ref(
    interleaved.data());
  const unsigned testIndices[] = {0, 1, 5, 6};
  static constexpr unsigned N_READS = 3;

  for (uint8_t ch = 0; ch < N_CHANNELS; ch++)
    for (unsigned index : testIndices) {
      ring.Seek(index, ch);
      ref.Seek(index, ch);
      for (unsigned readI = 0; readI < N_READS; readI++)
        GOAssert(
          ring.NextItem() == ref.NextItem(),
          "index=" + std::to_string(index) + " ch=" + std::to_string(ch)
            + " read=" + std::to_string(readI) + ": must match PtrFrameVector");
    }
}

void GOTestSoundResample::TestRingMirroredSeamReads() {
  static constexpr uint8_t N_CHANNELS = 2;
  static constexpr unsigned N_RING_FRAMES = 12;
  static constexpr unsigned N_POINTS = GOSoundResample::POLYPHASE_POINTS;
  static constexpr unsigned N_TAIL_FRAMES = N_POINTS;
  static constexpr unsigned N_PHYSICAL_FRAMES = N_RING_FRAMES + N_TAIL_FRAMES;

  std::vector<float> planar(N_CHANNELS * N_PHYSICAL_FRAMES);

  for (uint8_t ch = 0; ch < N_CHANNELS; ch++) {
    for (unsigned frameI = 0; frameI < N_RING_FRAMES; frameI++)
      planar[ch * N_PHYSICAL_FRAMES + frameI] = ch * 100.0f + frameI;
    // Mirror the first N_POINTS logical frames (wrapped) into the tail, as
    // the documented contract requires the ring owner to do.
    for (unsigned pointI = 0; pointI < N_POINTS; pointI++)
      planar[ch * N_PHYSICAL_FRAMES + N_RING_FRAMES + pointI]
        = ch * 100.0f + (pointI % N_RING_FRAMES);
  }

  GOSoundResample::RingPlanarFrameVector<float, float> ring(
    planar.data(), N_CHANNELS, N_RING_FRAMES, N_TAIL_FRAMES);
  // Starting indices whose N_POINTS-wide read crosses the seam (the seam
  // itself is at index == N_RING_FRAMES; Seek() only accepts indices below
  // that, so the highest start tested is N_RING_FRAMES - 1, whose read runs
  // entirely past the seam except for its first item).
  const unsigned startIndices[] = {
    N_RING_FRAMES - 7,
    N_RING_FRAMES - 6,
    N_RING_FRAMES - 5,
    N_RING_FRAMES - 4,
    N_RING_FRAMES - 3,
    N_RING_FRAMES - 2,
    N_RING_FRAMES - 1};

  for (uint8_t ch = 0; ch < N_CHANNELS; ch++)
    for (unsigned startIndex : startIndices) {
      ring.Seek(startIndex, ch);
      for (unsigned pointI = 0; pointI < N_POINTS; pointI++) {
        const float expected
          = ch * 100.0f + ((startIndex + pointI) % N_RING_FRAMES);

        GOAssert(
          ring.NextItem() == expected,
          "ch=" + std::to_string(ch) + " start=" + std::to_string(startIndex)
            + " step=" + std::to_string(pointI) + ": must read wrapped value");
      }
    }
}

void GOTestSoundResample::TestRingNormalizePosition() {
  static constexpr unsigned UPSAMPLE_FACTOR = GOSoundResample::UPSAMPLE_FACTOR;
  static constexpr unsigned N_POINTS = GOSoundResample::POLYPHASE_POINTS;

  // Growth of 1 index per Inc() call (fractionIncrement == UPSAMPLE_FACTOR),
  // small ring, enough steps to wrap several times. The ring's data buffer
  // itself is never read here - only NormalizePosition() is exercised. The
  // physical size still follows the class's real margin contract (logical
  // size + N_POINTS), like every other test's ring, even though nothing
  // here reads into that margin.
  {
    static constexpr unsigned N_RING_FRAMES = 5;
    static constexpr unsigned N_TAIL_FRAMES = N_POINTS;
    static constexpr unsigned N_PHYSICAL_FRAMES = N_RING_FRAMES + N_TAIL_FRAMES;
    static constexpr unsigned N_STEPS = 17;

    std::vector<float> planar(N_PHYSICAL_FRAMES);
    GOSoundResample::RingPlanarFrameVector<float, float> ring(
      planar.data(), 1, N_RING_FRAMES, N_TAIL_FRAMES);
    GOSoundResample::ResamplingPosition pos;

    pos.Init(1.0f);
    for (unsigned step = 1; step <= N_STEPS; step++) {
      pos.Inc(UPSAMPLE_FACTOR);
      ring.NormalizePosition(pos);
      GOAssert(
        pos.GetIndex() == step % N_RING_FRAMES,
        "growth=1 step " + std::to_string(step)
          + ": index must match (unbounded index) % nRingFrames");
    }
  }

  // Growth of 2 indices per Inc() call, a different ring size - confirms
  // the single-subtract correction also holds for a larger per-call step.
  {
    static constexpr unsigned N_RING_FRAMES = 7;
    static constexpr unsigned N_TAIL_FRAMES = N_POINTS;
    static constexpr unsigned N_PHYSICAL_FRAMES = N_RING_FRAMES + N_TAIL_FRAMES;
    static constexpr unsigned N_STEPS = 20;

    std::vector<float> planar(N_PHYSICAL_FRAMES);
    GOSoundResample::RingPlanarFrameVector<float, float> ring(
      planar.data(), 1, N_RING_FRAMES, N_TAIL_FRAMES);
    GOSoundResample::ResamplingPosition pos;

    pos.Init(1.0f);
    for (unsigned step = 1; step <= N_STEPS; step++) {
      pos.Inc(UPSAMPLE_FACTOR * 2);
      ring.NormalizePosition(pos);
      GOAssert(
        pos.GetIndex() == (2 * step) % N_RING_FRAMES,
        "growth=2 step " + std::to_string(step)
          + ": index must match (unbounded index) % nRingFrames");
    }
  }
}

void GOTestSoundResample::TestRingResamplerIntegration() {
  static constexpr uint8_t N_CHANNELS = 2;
  static constexpr unsigned N_RING_FRAMES = 12;
  static constexpr unsigned N_POINTS = GOSoundResample::POLYPHASE_POINTS;
  static constexpr unsigned N_TAIL_FRAMES = N_POINTS;
  static constexpr unsigned N_PHYSICAL_FRAMES = N_RING_FRAMES + N_TAIL_FRAMES;
  static constexpr unsigned N_OUT_FRAMES = 10;
  static constexpr unsigned N_OUT_ITEMS = N_OUT_FRAMES * N_CHANNELS;
  static constexpr unsigned FLAT_SIZE = 64;
  static constexpr unsigned START_INDEX = 8;
  static constexpr unsigned UPSAMPLE_FACTOR = GOSoundResample::UPSAMPLE_FACTOR;

  // A per-channel signal that repeats every N_RING_FRAMES frames - the
  // ring's planar (with mirrored tail) and the flat reference's "manually
  // unwrapped" layout are two different representations of the same
  // logical values. The per-channel offset means a channel-stride bug
  // (e.g. reading channel 1 from channel 0's own tail mirror) shows up as
  // a value mismatch, not a coincidentally-equal one.
  auto logicalValue = [](uint8_t ch, unsigned frameI) {
    return (float)(ch * 100 + frameI % N_RING_FRAMES);
  };

  std::vector<float> planar(N_CHANNELS * N_PHYSICAL_FRAMES);

  for (uint8_t ch = 0; ch < N_CHANNELS; ch++) {
    for (unsigned frameI = 0; frameI < N_RING_FRAMES; frameI++)
      planar[ch * N_PHYSICAL_FRAMES + frameI] = logicalValue(ch, frameI);
    for (unsigned pointI = 0; pointI < N_POINTS; pointI++)
      planar[ch * N_PHYSICAL_FRAMES + N_RING_FRAMES + pointI]
        = logicalValue(ch, pointI);
  }

  std::vector<float> flat(FLAT_SIZE * N_CHANNELS);

  for (unsigned frameI = 0; frameI < FLAT_SIZE; frameI++)
    for (uint8_t ch = 0; ch < N_CHANNELS; ch++)
      flat[frameI * N_CHANNELS + ch] = logicalValue(ch, frameI);

  // Increments varying around 1x - large enough on average that the
  // resampling position crosses the ring's seam and wraps at least once
  // over N_OUT_FRAMES, starting close to the seam (START_INDEX).
  const unsigned increments[N_OUT_FRAMES] = {
    UPSAMPLE_FACTOR,
    UPSAMPLE_FACTOR * 3 / 2,
    UPSAMPLE_FACTOR / 2,
    UPSAMPLE_FACTOR,
    UPSAMPLE_FACTOR * 5 / 4,
    UPSAMPLE_FACTOR,
    UPSAMPLE_FACTOR * 3 / 4,
    UPSAMPLE_FACTOR * 3 / 2,
    UPSAMPLE_FACTOR,
    UPSAMPLE_FACTOR};

  GOSoundResample::RingPlanarFrameVector<float, float> fvRing(
    planar.data(), N_CHANNELS, N_RING_FRAMES, N_TAIL_FRAMES);
  GOSoundResample::PtrFrameVector<float, float, N_CHANNELS> fvFlat(flat.data());
  GOSoundResample::ResamplingPosition posRing;
  GOSoundResample::ResamplingPosition posFlat;

  posRing.Init(1.0f, START_INDEX);
  posFlat.Init(1.0f, START_INDEX);

  GOSoundResample resampler;
  GOSoundResample::PolyphaseResampler polyphaseResampler(resampler);
  // Planar layout: channel c's frame f is at output[c * N_OUT_FRAMES + f].
  std::vector<float> outRing(N_OUT_ITEMS);
  std::vector<float> outFlat(N_OUT_ITEMS);

  polyphaseResampler.ResampleBlockVariableRatePlanar<
    GOSoundResample::RingPlanarFrameVector<float, float>>(
    posRing,
    fvRing,
    increments,
    N_OUT_FRAMES,
    N_CHANNELS,
    outRing.data(),
    N_OUT_FRAMES);
  polyphaseResampler.ResampleBlockVariableRatePlanar<
    GOSoundResample::PtrFrameVector<float, float, N_CHANNELS>>(
    posFlat,
    fvFlat,
    increments,
    N_OUT_FRAMES,
    N_CHANNELS,
    outFlat.data(),
    N_OUT_FRAMES);

  GOAssert(
    posFlat.GetIndex() >= N_RING_FRAMES,
    "test setup must actually cross the ring's seam at least once");
  for (unsigned itemI = 0; itemI < N_OUT_ITEMS; itemI++)
    GOAssert(
      are_close(outFlat[itemI], outRing[itemI]),
      "item " + std::to_string(itemI)
        + ": RingPlanarFrameVector output must match PtrFrameVector output "
          "(within tolerance) over the equivalent unwrapped source");
  GOAssert(
    posRing.GetIndex() == posFlat.GetIndex() % N_RING_FRAMES,
    "final ring index must match the unwrapped index modulo nRingFrames");
}

void GOTestSoundResample::TestRateToFractionIncrement() {
  static constexpr unsigned UPSAMPLE_FACTOR = GOSoundResample::UPSAMPLE_FACTOR;

  // Usable at compile time
  static_assert(
    GOSoundResample::rateToFractionIncrement(1.0f) == UPSAMPLE_FACTOR);
  static_assert(
    GOSoundResample::rateToFractionIncrement(0.125f) == UPSAMPLE_FACTOR / 8);
  static_assert(
    GOSoundResample::rateToFractionIncrement(8.0f) == UPSAMPLE_FACTOR * 8);

  const float usedRates[] = {0.125f, 0.25f, 1.0f, 4.0f, 8.0f};

  for (float rate : usedRates)
    GOAssert(
      GOSoundResample::rateToFractionIncrement(rate)
        == (unsigned)roundf(rate * UPSAMPLE_FACTOR),
      "rate " + std::to_string(rate) + " must round as roundf() did");

  // A grid of non-negative rates, including cents-derived ones
  for (unsigned i = 0; i <= 4000; i++) {
    const float rate = 0.0001f + i * 0.002f;

    GOAssert(
      GOSoundResample::rateToFractionIncrement(rate)
        == (unsigned)roundf(rate * UPSAMPLE_FACTOR),
      "rate " + std::to_string(rate) + " must round as roundf() did");
  }
  for (int cents = -2400; cents <= 2400; cents += 7) {
    const float rate = std::exp2(cents / 1200.0f);

    GOAssert(
      GOSoundResample::rateToFractionIncrement(rate)
        == (unsigned)roundf(rate * UPSAMPLE_FACTOR),
      "cents " + std::to_string(cents) + " must round as roundf() did");
  }
}

void GOTestSoundResample::TestGetIndexIncrementByUnits() {
  static constexpr unsigned UPSAMPLE_FACTOR = GOSoundResample::UPSAMPLE_FACTOR;

  const unsigned startFractions[]
    = {0, 1, UPSAMPLE_FACTOR / 2, UPSAMPLE_FACTOR - 1};
  unsigned seed = 12345;

  for (unsigned startFraction : startFractions) {
    for (unsigned iteration = 0; iteration < 200; iteration++) {
      GOSoundResample::ResamplingPosition pos;
      uint64_t nUnits = 0;

      pos.Init(1.0f, 100);
      // Set the starting fraction by one Inc() from the zero fraction
      pos.Inc(startFraction);

      const unsigned startIndex = pos.GetIndex();

      GOAssert(pos.GetFraction() == startFraction, "the starting fraction");
      for (unsigned stepI = 0; stepI < 1 + iteration % 50; stepI++) {
        seed = seed * 1103515245u + 12345u;

        const unsigned increment = (seed >> 8) % (8 * UPSAMPLE_FACTOR + 1);

        pos.Inc(increment);
        nUnits += increment;
        GOAssert(
          GOSoundResample::getIndexIncrementByUnits(startFraction, nUnits)
            == pos.GetIndex() - startIndex,
          "the index growth must match after " + std::to_string(stepI + 1)
            + " increments");
      }
    }
  }
}

void GOTestSoundResample::TestComputeNUnitsToReach() {
  static constexpr unsigned UPSAMPLE_FACTOR = GOSoundResample::UPSAMPLE_FACTOR;

  const unsigned fractions[] = {0, 1, UPSAMPLE_FACTOR - 1};
  const unsigned nFramesCases[] = {1, 2, 10};

  for (unsigned fraction : fractions) {
    GOAssert(
      GOSoundResample::computeMinNUnitsToReach(fraction, 0) == 0,
      "computeMinNUnitsToReach(f, 0) must be 0");
    for (unsigned k : nFramesCases) {
      const uint64_t minUnits
        = GOSoundResample::computeMinNUnitsToReach(fraction, k);
      const uint64_t maxUnits
        = GOSoundResample::computeMaxNUnitsToReach(fraction, k);
      const std::string caseName
        = " (f=" + std::to_string(fraction) + ", k=" + std::to_string(k) + ")";

      GOAssert(minUnits == maxUnits, "both helpers give k * F - f" + caseName);

      // Inc()-ing exactly that many units lands on index + k, fraction 0
      GOSoundResample::ResamplingPosition pos;

      pos.Init(1.0f, 0);
      pos.Inc(fraction);

      const unsigned startIndex = pos.GetIndex();

      pos.Inc((unsigned)minUnits);
      GOAssert(
        pos.GetIndex() == startIndex + k && pos.GetFraction() == 0,
        "the exact sum lands on index + k with a zero fraction" + caseName);

      // One unit fewer: one frame short (min) / still within the bound (max)
      GOAssert(
        GOSoundResample::getIndexIncrementByUnits(fraction, minUnits - 1)
          == k - 1,
        "one unit fewer stays one frame short" + caseName);
      // One unit more: the minimum is exceeded, the maximum bound is passed
      GOAssert(
        GOSoundResample::getIndexIncrementByUnits(fraction, minUnits + 1) >= k,
        "one unit more reaches k frames" + caseName);
      GOAssert(
        GOSoundResample::getIndexIncrementByUnits(fraction, maxUnits + 1) == k,
        "one unit more than the maximum is past index + k" + caseName);
    }
  }

  // The equivalences on a grid of f, n, k
  for (unsigned fraction = 0; fraction < UPSAMPLE_FACTOR; fraction += 1021)
    for (unsigned k = 1; k <= 5; k++)
      for (uint64_t n = 0; n < 6 * (uint64_t)UPSAMPLE_FACTOR; n += 997) {
        const unsigned nFramesAdvanced
          = GOSoundResample::getIndexIncrementByUnits(fraction, n);
        const bool isBeforeOrOn = nFramesAdvanced < k
          || (nFramesAdvanced == k
              && ((fraction + n) & GOSoundResample::UPSAMPLE_MASK) == 0);

        GOAssert(
          (nFramesAdvanced >= k)
            == (n >= GOSoundResample::computeMinNUnitsToReach(fraction, k)),
          "min equivalence on the grid");
        GOAssert(
          isBeforeOrOn
            == (n <= GOSoundResample::computeMaxNUnitsToReach(fraction, k)),
          "max equivalence on the grid");
      }
}

void GOTestSoundResample::run() {
  TestConstantRateEquivalence();
  TestVaryingRateCorrectness();
  TestRingNonWrappingReads();
  TestRingMirroredSeamReads();
  TestRingNormalizePosition();
  TestRingResamplerIntegration();
  TestRateToFractionIncrement();
  TestGetIndexIncrementByUnits();
  TestComputeNUnitsToReach();
}
