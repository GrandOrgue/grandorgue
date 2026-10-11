/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundLinearFaderCore.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <vector>

#include "sound/buffer/GOSoundBufferPlanar.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"

#include "GOTestScope.h"

const std::string GOTestSoundLinearFaderCore::TEST_NAME
  = "GOTestSoundLinearFaderCore";

static constexpr float ZERO = GOSoundLinearFaderCore::GAIN_ZERO;
static constexpr float ONE = GOSoundLinearFaderCore::GAIN_ONE;
static constexpr float TOLERANCE = 1e-5f;

static bool is_near(float a, float b) { return std::fabs(a - b) <= TOLERANCE; }

static bool is_bit_equal(
  const std::vector<float> &a, const std::vector<float> &b) {
  return a.size() == b.size()
    && std::memcmp(a.data(), b.data(), a.size() * sizeof(float)) == 0;
}

void GOTestSoundLinearFaderCore::TestGainRampConstruction() {
  const GainRamp defaultRamp;

  GOAssert(defaultRamp.GetNRampFrames() == 0, "default: no frames");
  GOAssert(defaultRamp.GetDeltaPerFrame() == 0.0f, "default: delta 0");
  GOAssert(defaultRamp.GetEndGain() == ONE, "default: end gain 1");
  GOAssert(defaultRamp.GetStartGain() == ONE, "default: start gain 1");
  GOAssert(defaultRamp.GetGainAt(7) == ONE, "default: gain 1 everywhere");

  const GainRamp ramp(ZERO, ONE, 8);

  GOAssert(ramp.GetNRampFrames() == 8, "ramp keeps its frames");
  GOAssert(ramp.GetDeltaPerFrame() == 0.125f, "ramp delta");
  GOAssert(ramp.GetStartGain() == ZERO, "ramp start gain");
  GOAssert(ramp.GetEndGain() == ONE, "ramp end gain");

  // a whole ramp is assignable
  GainRamp assigned;

  assigned = ramp;
  GOAssert(
    assigned.GetNRampFrames() == 8 && assigned.GetGainAt(4) == 0.5f,
    "assignment");

  // 0 frames: constant-gain ramp, start gain unused
  const GainRamp constRamp(0.25f, 0.75f, 0);

  GOAssert(constRamp.GetNRampFrames() == 0, "0 frames: no frames");
  GOAssert(constRamp.GetDeltaPerFrame() == 0.0f, "0 frames: delta 0");
  GOAssert(constRamp.GetEndGain() == 0.75f, "0 frames: end gain stored");
  GOAssert(constRamp.GetGainAt(3) == 0.75f, "0 frames: gain is the end gain");
  GOAssert(
    constRamp.IsGainInRange(0.75f) && !constRamp.IsGainInRange(0.25f),
    "0 frames: only the own gain is in range");

  // normalisation: start == end makes a constant-gain ramp
  const GainRamp flat(0.5f, 0.5f, 10);

  GOAssert(flat.GetNRampFrames() == 0, "flat: frames normalised to 0");
  GOAssert(flat.GetStartGain() == 0.5f, "flat: start gain normalised");
  GOAssert(flat.IsOverAt(0), "flat: over at 0");
  GOAssert(flat.GetNFramesInRamp(0, 10) == 0, "flat: no frames inside");
  GOAssert(flat.GetGainAt(5) == 0.5f, "flat: gain");
  GOAssert(flat.GetGainOffset(0.5f) == 0, "flat: offset 0");
}

void GOTestSoundLinearFaderCore::TestGainRampQueries() {
  const GainRamp ramp(ZERO, ONE, 8);

  GOAssert(ramp.GetGainAt(0) == 0.0f, "gain at 0");
  GOAssert(ramp.GetGainAt(4) == 0.5f, "gain at 4");
  GOAssert(ramp.GetGainAt(7) == 0.875f, "gain at 7");
  GOAssert(ramp.GetGainAt(8) == 1.0f, "gain at the end");
  GOAssert(ramp.GetGainAt(100) == 1.0f, "gain past the end");
  GOAssert(ramp.GetGainInside(3) == 0.375f, "gain inside");
  GOAssert(!ramp.IsOverAt(0) && !ramp.IsOverAt(7), "not over inside");
  GOAssert(ramp.IsOverAt(8) && ramp.IsOverAt(9), "over at and past the end");
  GOAssert(ramp.GetNFramesInRamp(0, 4) == 4, "piece inside");
  GOAssert(ramp.GetNFramesInRamp(6, 4) == 2, "piece across the end");
  GOAssert(ramp.GetNFramesInRamp(8, 4) == 0, "piece at the end");
  GOAssert(ramp.GetNFramesInRamp(1000, 4) == 0, "piece far past the end");

  const GainRamp falling(ONE, 0.5f, 4);

  GOAssert(falling.GetGainAt(2) == 0.75f, "falling gain");
  GOAssert(falling.GetGainAt(4) == 0.5f, "falling end gain");
}

void GOTestSoundLinearFaderCore::TestGainRampGainSum() {
  const GainRamp ramps[] = {
    GainRamp(ZERO, ONE, 64),
    GainRamp(ONE, ZERO, 64),
    GainRamp(0.25f, 0.75f, 50),
    GainRamp(0.5f, 0.5f, 0)};
  const unsigned positions[] = {0, 1, 2, 17, 63, 64, 65, 200};

  for (const GainRamp &ramp : ramps)
    for (unsigned frameI : positions) {
      double directSum = 0.0;

      for (unsigned i = 0; i < frameI; ++i)
        directSum += ramp.GetGainAt(i);
      GOAssert(
        std::fabs(ramp.GetGainSumBefore(frameI) - directSum) < 1e-3,
        "the sum equals the sum of the gains: frame " + std::to_string(frameI));
    }

  const GainRamp rising(ZERO, ONE, 64);
  const GainRamp constant(0.5f, 0.5f, 0);

  GOAssert(rising.GetGainSumBefore(0) == 0.0, "an empty sum is 0");
  GOAssert(
    std::fabs(
      rising.GetGainSumBefore(74) - (rising.GetGainSumBefore(64) + 10.0))
      < 1e-3,
    "past the end of the ramp the end gain is summed");
  GOAssert(
    std::fabs(constant.GetGainSumBefore(100) - 50.0) < 1e-9,
    "a constant-gain ramp gives frameI * gain");
}

void GOTestSoundLinearFaderCore::TestGainOffset() {
  const GainRamp rising(ZERO, ONE, 8);
  const GainRamp falling(ONE, 0.5f, 4);

  GOAssert(
    rising.IsGainInRange(0.0f) && rising.IsGainInRange(1.0f), "range ends");
  GOAssert(
    rising.IsGainInRange(-0.0625f) && rising.IsGainInRange(1.0625f),
    "range slack");
  GOAssert(
    !rising.IsGainInRange(-0.07f) && !rising.IsGainInRange(1.07f),
    "outside the range");
  GOAssert(
    falling.IsGainInRange(0.5f) && falling.IsGainInRange(1.0f)
      && !falling.IsGainInRange(0.3f) && !falling.IsGainInRange(1.2f),
    "falling range");

  for (unsigned frameI = 0; frameI <= 8; ++frameI)
    GOAssert(
      rising.GetGainOffset(rising.GetGainAt(frameI)) == frameI,
      "rising round trip " + std::to_string(frameI));
  for (unsigned frameI = 0; frameI <= 4; ++frameI)
    GOAssert(
      falling.GetGainOffset(falling.GetGainAt(frameI)) == frameI,
      "falling round trip " + std::to_string(frameI));
  GOAssert(rising.GetGainOffset(0.51f) == 4, "rounding to the nearest frame");
  GOAssert(rising.GetGainOffset(-0.0625f) == 0, "slack clamps to 0");
  GOAssert(rising.GetGainOffset(1.0625f) == 8, "slack clamps to the end");
}

void GOTestSoundLinearFaderCore::TestFade() {
  const GainRamp ramp(ZERO, ONE, 8);

  {
    std::vector<float> data(8, 1.0f);

    Core::fade<1>(ramp, 0, 8, data.data());
    for (unsigned frameI = 0; frameI < 8; ++frameI)
      GOAssert(is_near(data[frameI], frameI * 0.125f), "fade<1> values");
  }
  {
    std::vector<float> data(16, 1.0f);

    Core::fade<2>(ramp, 0, 8, data.data());
    for (unsigned frameI = 0; frameI < 8; ++frameI)
      GOAssert(
        is_near(data[frameI * 2], frameI * 0.125f)
          && data[frameI * 2] == data[frameI * 2 + 1],
        "fade<2> equals fade<1> per channel");
  }
  {
    // pieces equal one call bit-for-bit, also across and past the ramp end
    std::vector<float> whole(24, 1.0f);
    std::vector<float> pieces(24, 1.0f);

    Core::fade<1>(ramp, 0, 24, whole.data());
    Core::fade<1>(ramp, 0, 5, pieces.data());
    Core::fade<1>(ramp, 5, 7, pieces.data() + 5);
    Core::fade<1>(ramp, 12, 12, pieces.data() + 12);
    GOAssert(is_bit_equal(whole, pieces), "consecutive pieces");
    GOAssert(whole[8] == 1.0f && whole[23] == 1.0f, "past the end: end gain");
    GOAssert(
      pieces[8] == 1.0f && pieces[23] == 1.0f, "pieces past the end: end gain");
  }
  {
    // far past the end, end gain 0.5
    const GainRamp half(ONE, 0.5f, 4);
    std::vector<float> data(4, 1.0f);

    Core::fade<1>(half, 1000, 4, data.data());
    GOAssert(is_bit_equal(data, {0.5f, 0.5f, 0.5f, 0.5f}), "far past the end");
  }
  {
    // a constant-gain ramp at any offset
    const GainRamp constRamp(ZERO, 0.5f, 0);
    std::vector<float> data(4, 1.0f);

    Core::fade<2>(constRamp, 3, 2, data.data());
    GOAssert(is_bit_equal(data, {0.5f, 0.5f, 0.5f, 0.5f}), "constant ramp");
  }
}

void GOTestSoundLinearFaderCore::TestFadePlanar() {
  // a ramp across the piece: inside, then past the end
  const GainRamp ramp(ONE, 0.25f, 6);

  for (unsigned nChannels = 1; nChannels <= 3; ++nChannels) {
    const unsigned nFrames = 10;
    std::vector<float> data(nChannels * nFrames);

    for (unsigned itemI = 0; itemI < data.size(); ++itemI)
      data[itemI] = 1.0f + 0.5f * itemI;

    std::vector<float> expected = data;

    for (unsigned channelI = 0; channelI < nChannels; ++channelI)
      Core::fade<1>(ramp, 2, nFrames, expected.data() + channelI * nFrames);

    GOSoundBufferPlanarMutable buffer(data.data(), nChannels, nFrames);

    Core::fadePlanar(ramp, 2, buffer);
    GOAssert(
      is_bit_equal(data, expected),
      "fadePlanar equals fade<1> per channel, channels "
        + std::to_string(nChannels));
  }
  {
    // a constant-gain ramp of the gain 0 silences all channels
    std::vector<float> data(2 * 4, 3.0f);
    GOSoundBufferPlanarMutable buffer(data.data(), 2, 4);

    Core::fadePlanar(GainRamp(ZERO, ZERO, 0), 0, buffer);
    GOAssert(is_bit_equal(data, std::vector<float>(8, 0.0f)), "gain 0");
  }
}

void GOTestSoundLinearFaderCore::TestFadeConstantDispatch() {
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float denormal = std::numeric_limits<float>::denorm_min();

  // GAIN_ZERO: exact zeros even for NaN / denormals
  {
    std::vector<float> data = {nan, denormal, 1.0f, -2.0f};

    Core::fade<2>(GainRamp(ZERO, ZERO, 0), 0, 2, data.data());
    GOAssert(is_bit_equal(data, {0, 0, 0, 0}), "constant 0: zeros");
  }
  // GAIN_ONE: not touched
  {
    std::vector<float> data = {nan, denormal, 1.0f, -2.0f};
    const std::vector<float> orig = data;

    Core::fade<2>(GainRamp(ONE, ONE, 0), 0, 2, data.data());
    GOAssert(is_bit_equal(data, orig), "constant 1: untouched");
  }
  // other: multiplied
  {
    std::vector<float> data = {1.0f, 2.0f, 4.0f, 8.0f};

    Core::fade<2>(GainRamp(0.25f, 0.25f, 0), 0, 2, data.data());
    GOAssert(
      is_bit_equal(data, {0.25f, 0.5f, 1.0f, 2.0f}),
      "constant 0.25: multiplied");
  }
  // the tail after a real ramp: end gain 0 / 1 / 0.5
  {
    std::vector<float> data = {nan, nan, nan, nan};

    Core::fade<1>(GainRamp(ONE, ZERO, 2), 0, 4, data.data());
    GOAssert(data[2] == 0.0f && data[3] == 0.0f, "tail 0: zeros");

    std::vector<float> data1 = {1, 1, 3, nan};

    Core::fade<1>(GainRamp(ZERO, ONE, 2), 0, 4, data1.data());
    // (std::isnan() may be optimised away by -ffast-math: compare the bits)
    GOAssert(
      data1[2] == 3.0f && std::memcmp(&data1[3], &nan, sizeof(float)) == 0,
      "tail 1: untouched");

    std::vector<float> data2 = {1, 1, 4, 8};

    Core::fade<1>(GainRamp(ONE, 0.5f, 2), 0, 4, data2.data());
    GOAssert(data2[2] == 2.0f && data2[3] == 4.0f, "tail 0.5: multiplied");
  }
}

void GOTestSoundLinearFaderCore::TestUnitRamp() {
  const Core::UnitRamp rising(true, 8);
  const Core::UnitRamp falling(false, 4);
  const Core::UnitRamp risingConst(true, 0);
  const Core::UnitRamp fallingConst(false, 0);
  const Core::UnitRamp defaultRamp;

  GOAssert(
    rising.GetStartGain() == ZERO && rising.GetEndGain() == ONE
      && rising.GetNRampFrames() == 8 && rising.IsRising(),
    "rising: 0 -> 1");
  GOAssert(
    falling.GetStartGain() == ONE && falling.GetEndGain() == ZERO
      && falling.GetNRampFrames() == 4 && !falling.IsRising(),
    "falling: 1 -> 0");
  GOAssert(
    risingConst.GetGainAt(5) == ONE && risingConst.IsRising(),
    "rising without frames: the constant 1, still rising");
  GOAssert(
    fallingConst.GetGainAt(5) == ZERO && !fallingConst.IsRising(),
    "falling without frames: the constant 0, still falling");
  GOAssert(
    defaultRamp.GetNRampFrames() == 0 && defaultRamp.GetGainAt(0) == ONE
      && defaultRamp.IsRising(),
    "the default is UnitRamp(true, 0)");
  GOAssert(
    rising.IsRising() && rising.IsOverAt(8) && rising.GetGainAt(100) == ONE
      && falling.IsOverAt(4) && !falling.IsRising(),
    "IsRising() does not change after the end of the ramp");
}

void GOTestSoundLinearFaderCore::TestCrossfadeRamp() {
  const Core::CrossfadeRamp toOther(Core::TO_OTHER, 8);
  const Core::CrossfadeRamp fromOther(Core::FROM_OTHER, 4);
  const Core::CrossfadeRamp defaultRamp;
  const Core::CrossfadeRamp toZero(Core::TO_OTHER, 0);
  const Core::CrossfadeRamp fromZero(Core::FROM_OTHER, 0);

  GOAssert(
    toOther.GetStartGain() == ONE && toOther.GetEndGain() == ZERO
      && toOther.GetNRampFrames() == 8,
    "TO_OTHER 1 -> 0 (the weight of pInOut)");
  GOAssert(
    fromOther.GetStartGain() == ZERO && fromOther.GetEndGain() == ONE
      && fromOther.GetNRampFrames() == 4,
    "FROM_OTHER 0 -> 1 (the weight of pInOut)");
  GOAssert(
    defaultRamp.GetNRampFrames() == 0 && defaultRamp.GetEndGain() == ZERO,
    "default is (TO_OTHER, 0)");
  GOAssert(
    toZero.GetGainAt(5) == ZERO && fromZero.GetGainAt(5) == ONE,
    "0 frames: constant 0 / 1");
  GOAssert(
    fromOther.IsRising() && !toOther.IsRising() && fromZero.IsRising()
      && !toZero.IsRising() && !defaultRamp.IsRising(),
    "IsRising(): FROM_OTHER yes, TO_OTHER no, also without frames");

  // accepted by GainRamp functions through the base reference
  std::vector<float> data(8, 1.0f);

  Core::fade<1>(toOther, 0, 8, data.data());
  GOAssert(is_near(data[4], 0.5f), "fade accepts a CrossfadeRamp");
  GOAssert(
    Core::mapFrameOffset(toOther, 4, fromOther) == 2,
    "mapFrameOffset accepts CrossfadeRamps");
}

void GOTestSoundLinearFaderCore::TestCrossfade() {
  const std::vector<float> other(10, 1.0f);

  for (Core::CrossfadeDirection direction :
       {Core::TO_OTHER, Core::FROM_OTHER}) {
    const Core::CrossfadeRamp ramp(direction, 8);
    std::vector<float> inOut(10, 0.0f);

    Core::crossfade<1>(ramp, 0, 10, other.data(), inOut.data());
    for (unsigned frameI = 0; frameI < 8; ++frameI) {
      // inOut is 0 and other is 1: the result is the weight of other
      const float weightOfOther = 1.0f - ramp.GetGainAt(frameI);

      GOAssert(is_near(inOut[frameI], weightOfOther), "crossfade<1> values");
    }
    // the tail is exact
    const float expectedTail = direction == Core::TO_OTHER ? 1.0f : 0.0f;

    GOAssert(
      inOut[8] == expectedTail && inOut[9] == expectedTail,
      "crossfade tail is exact");
  }
  {
    // <2> equals <1> per channel; pieces equal one call; pOther untouched
    const Core::CrossfadeRamp ramp(Core::TO_OTHER, 8);
    std::vector<float> otherStereo(20);
    std::vector<float> inOut2(20, 0.25f);
    std::vector<float> inOut1a(10, 0.25f);

    for (unsigned itemI = 0; itemI < 20; ++itemI)
      otherStereo[itemI] = 1.0f + itemI * 0.5f;

    const std::vector<float> otherCopy = otherStereo;
    std::vector<float> otherCh0(10);

    for (unsigned frameI = 0; frameI < 10; ++frameI)
      otherCh0[frameI] = otherStereo[frameI * 2];
    Core::crossfade<2>(ramp, 0, 10, otherStereo.data(), inOut2.data());
    Core::crossfade<1>(ramp, 0, 10, otherCh0.data(), inOut1a.data());
    for (unsigned frameI = 0; frameI < 10; ++frameI)
      GOAssert(inOut2[frameI * 2] == inOut1a[frameI], "<2> equals <1>");
    GOAssert(is_bit_equal(otherStereo, otherCopy), "pOther untouched");

    std::vector<float> whole(10, 0.25f);
    std::vector<float> pieces(10, 0.25f);

    Core::crossfade<1>(ramp, 0, 10, otherCh0.data(), whole.data());
    Core::crossfade<1>(ramp, 0, 3, otherCh0.data(), pieces.data());
    Core::crossfade<1>(ramp, 3, 4, otherCh0.data() + 3, pieces.data() + 3);
    Core::crossfade<1>(ramp, 7, 3, otherCh0.data() + 7, pieces.data() + 7);
    GOAssert(is_bit_equal(whole, pieces), "consecutive pieces");

    // far past the end and 0-frame ramps
    std::vector<float> far(4, 0.25f);

    Core::crossfade<1>(ramp, 1000, 4, otherCh0.data(), far.data());
    GOAssert(
      is_bit_equal(far, {otherCh0[0], otherCh0[1], otherCh0[2], otherCh0[3]}),
      "far past the end: pOther");

    std::vector<float> keep(4, 0.25f);

    Core::crossfade<1>(
      Core::CrossfadeRamp(Core::FROM_OTHER, 0),
      0,
      4,
      otherCh0.data(),
      keep.data());
    GOAssert(
      is_bit_equal(keep, {0.25f, 0.25f, 0.25f, 0.25f}), "0 frames: keep");
    Core::crossfade<1>(
      Core::CrossfadeRamp(Core::TO_OTHER, 0),
      0,
      4,
      otherCh0.data(),
      keep.data());
    GOAssert(
      is_bit_equal(keep, {otherCh0[0], otherCh0[1], otherCh0[2], otherCh0[3]}),
      "0 frames: take");
  }
  {
    // the in-place signal gets the same gain as in fade() for the same pieces
    // (the other signal is silence)
    for (Core::CrossfadeDirection direction :
         {Core::TO_OTHER, Core::FROM_OTHER}) {
      const Core::CrossfadeRamp ramp(direction, 8);
      std::vector<float> faded(8, 1.0f);
      std::vector<float> mixed(8, 1.0f);
      const std::vector<float> zeros(8, 0.0f);

      Core::fade<1>(ramp, 0, 8, faded.data());
      Core::crossfade<1>(ramp, 0, 8, zeros.data(), mixed.data());
      for (unsigned frameI = 0; frameI < 8; ++frameI)
        GOAssert(is_near(faded[frameI], mixed[frameI]), "same gain as fade");
    }
  }
}

void GOTestSoundLinearFaderCore::TestCrossfadePlanar() {
  for (unsigned nChannels = 1; nChannels <= 3; ++nChannels) {
    const unsigned nFrames = 10;
    const Core::CrossfadeRamp ramp(Core::FROM_OTHER, 6);
    std::vector<float> other(nChannels * nFrames);
    std::vector<float> inOut(nChannels * nFrames);

    for (unsigned itemI = 0; itemI < other.size(); ++itemI) {
      other[itemI] = 1.0f + itemI;
      inOut[itemI] = -0.5f * itemI;
    }

    std::vector<float> expected = inOut;

    for (unsigned channelI = 0; channelI < nChannels; ++channelI)
      Core::crossfade<1>(
        ramp,
        2,
        nFrames,
        other.data() + channelI * nFrames,
        expected.data() + channelI * nFrames);

    const GOSoundBufferPlanar otherBuffer(other.data(), nChannels, nFrames);
    GOSoundBufferPlanarMutable inOutBuffer(inOut.data(), nChannels, nFrames);

    Core::crossfadePlanar(ramp, 2, otherBuffer, inOutBuffer);
    GOAssert(
      is_bit_equal(inOut, expected),
      "crossfadePlanar, channels " + std::to_string(nChannels));
  }
}

void GOTestSoundLinearFaderCore::TestMapFrameOffset() {
  const Core::CrossfadeRamp up(Core::TO_OTHER, 8);
  const Core::CrossfadeRamp down(Core::FROM_OTHER, 4);

  // the weight 0.5 is at 4 in up and at 2 in down
  GOAssert(Core::canMapFrameOffset(up, 4, down), "up -> down can be mapped");
  GOAssert(Core::mapFrameOffset(up, 4, down) == 2, "up -> down");
  GOAssert(Core::mapFrameOffset(down, 2, up) == 4, "down -> up");
  GOAssert(Core::mapFrameOffset(up, 0, down) == 4, "gain 1 -> end of down");
  GOAssert(Core::mapFrameOffset(up, 8, down) == 0, "gain 0 -> start of down");
  GOAssert(Core::mapFrameOffset(up, 100, down) == 0, "past the end of up");
  for (unsigned frameI = 0; frameI <= 8; ++frameI) {
    const unsigned there = Core::mapFrameOffset(up, frameI, down);
    const unsigned back = Core::mapFrameOffset(down, there, up);

    GOAssert(
      (back > frameI ? back - frameI : frameI - back) <= 2,
      "round trip " + std::to_string(frameI));
  }

  // general ramps with different gain ranges
  const GainRamp narrow(0.25f, 0.75f, 4);
  const GainRamp wide(ZERO, ONE, 8);

  GOAssert(Core::canMapFrameOffset(narrow, 2, wide), "inside the range");
  GOAssert(Core::mapFrameOffset(narrow, 2, wide) == 4, "narrow -> wide");
  GOAssert(!Core::canMapFrameOffset(wide, 0, narrow), "gain 0 out of narrow");
  GOAssert(!Core::canMapFrameOffset(wide, 8, narrow), "gain 1 out of narrow");

  // constant-gain ramps
  const GainRamp constHalf(0.5f, 0.5f, 0);
  const GainRamp constOne(ONE, ONE, 0);

  GOAssert(Core::canMapFrameOffset(constHalf, 0, up), "leave a constant ramp");
  GOAssert(Core::mapFrameOffset(constHalf, 7, up) == 4, "leave: end gain");
  GOAssert(
    !Core::canMapFrameOffset(up, 2, constHalf),
    "a constant target with another gain");
  GOAssert(
    Core::canMapFrameOffset(up, 4, constHalf)
      && Core::mapFrameOffset(up, 4, constHalf) == 0,
    "a constant target with its own gain gives 0");
  GOAssert(
    Core::canMapFrameOffset(constOne, 0, constOne)
      && Core::mapFrameOffset(constOne, 0, constOne) == 0,
    "constant to constant");
}

void GOTestSoundLinearFaderCore::TestMapFrameOffsetOrJump() {
  const Core::UnitRamp rising(true, 8);
  const Core::UnitRamp falling(false, 4);
  const Core::UnitRamp risingConst(true, 0);
  const Core::UnitRamp fallingConst(false, 0);

  // a target with frames is exactly mapFrameOffset()
  GOAssert(
    Core::mapFrameOffsetOrJump(rising, 4, falling)
      == Core::mapFrameOffset(rising, 4, falling),
    "rising to falling, mid-ramp");
  GOAssert(
    Core::mapFrameOffsetOrJump(rising, 100, falling)
      == Core::mapFrameOffset(rising, 100, falling),
    "rising to falling, the source is finished");
  GOAssert(
    Core::mapFrameOffsetOrJump(falling, 2, rising)
      == Core::mapFrameOffset(falling, 2, rising),
    "falling to rising, mid-ramp");

  // a target without frames: 0 whatever its gain is, no assert
  GOAssert(
    !Core::canMapFrameOffset(rising, 2, fallingConst),
    "the gain is not that of the constant target");
  GOAssert(
    Core::mapFrameOffsetOrJump(rising, 2, fallingConst) == 0,
    "a constant target with another gain gives 0");
  GOAssert(
    Core::mapFrameOffsetOrJump(rising, 100, risingConst) == 0,
    "a constant target with its own gain gives 0");
  GOAssert(
    Core::mapFrameOffsetOrJump(risingConst, 0, falling)
      == Core::mapFrameOffset(risingConst, 0, falling),
    "a constant source");
}

void GOTestSoundLinearFaderCore::run() {
  GO_RUN_TEST(TestGainRampConstruction())
  GO_RUN_TEST(TestGainRampQueries())
  GO_RUN_TEST(TestGainRampGainSum())
  GO_RUN_TEST(TestGainOffset())
  GO_RUN_TEST(TestFade())
  GO_RUN_TEST(TestFadePlanar())
  GO_RUN_TEST(TestFadeConstantDispatch())
  GO_RUN_TEST(TestUnitRamp())
  GO_RUN_TEST(TestCrossfadeRamp())
  GO_RUN_TEST(TestCrossfade())
  GO_RUN_TEST(TestCrossfadePlanar())
  GO_RUN_TEST(TestMapFrameOffset())
  GO_RUN_TEST(TestMapFrameOffsetOrJump())
}
