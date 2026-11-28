/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundConversions.h"

#include <cmath>

#include "sound/dsp-kernels/GOSoundConversions.h"

#include "GOTestScope.h"

const std::string GOTestSoundConversions::TEST_NAME = "GOTestSoundConversions";

void GOTestSoundConversions::TestGain() {
  using Conv = GOSoundConversions;

  GOAssert(Conv::gainToVolume(0.0f) == 1.0f, "0 dB must give exactly 1");
  GOAssert(std::abs(Conv::gainToVolume(20.0f) - 10.0f) < 1e-5f, "+20 dB is 10");
  GOAssert(
    std::abs(Conv::gainToVolume(-20.0f) - 0.1f) < 1e-7f, "-20 dB is 0.1");
  GOAssert(
    Conv::gainToVolume(-6.5f) == powf(10.0f, -6.5f * 0.05f),
    "the result must be bit for bit the old expression");

  GOAssert(
    Conv::gainToVolumeOrMute(Conv::MIN_GAIN_DB)
      == Conv::gainToVolume(Conv::MIN_GAIN_DB),
    "the lowest gain is not muted");
  GOAssert(
    Conv::gainToVolumeOrMute(Conv::MAX_GAIN_DB)
      == Conv::gainToVolume(Conv::MAX_GAIN_DB),
    "the highest gain is not muted");
  GOAssert(
    Conv::gainToVolumeOrMute(Conv::MUTE_GAIN_DB) == 0.0f,
    "the mute gain gives silence");
  GOAssert(
    Conv::gainToVolumeOrMute(Conv::MIN_GAIN_DB - 0.5f) == 0.0f
      && Conv::gainToVolumeOrMute(Conv::MAX_GAIN_DB + 0.5f) == 0.0f,
    "a gain outside the limits gives silence");
}

void GOTestSoundConversions::TestCents() {
  using Conv = GOSoundConversions;

  GOAssert(Conv::centsToRatio(0.0) == 1.0, "0 cents is exactly 1");
  GOAssert(Conv::centsToRatio(1200.0) == 2.0, "1200 cents is an octave up");
  GOAssert(Conv::centsToRatio(-1200.0) == 0.5, "-1200 cents is an octave down");
  GOAssert(Conv::ratioToCents(1.0) == 0.0, "ratio 1 is 0 cents");
  GOAssert(Conv::ratioToCents(2.0) == 1200.0, "ratio 2 is 1200 cents");
  for (const double cents : {-2400.0, -37.5, -1.0, 0.25, 15.0, 100.0, 1900.0})
    GOAssert(
      std::abs(Conv::ratioToCents(Conv::centsToRatio(cents)) - cents) < 1e-9,
      "ratioToCents() must invert centsToRatio()");
}

void GOTestSoundConversions::TestMsToFrames() {
  using Conv = GOSoundConversions;

  GOAssert(
    Conv::msToFramesRounded(1000, 48000) == 48000
      && Conv::msToFramesCeil(1000, 48000) == 48000,
    "an exact duration must not be changed by any rounding");
  GOAssert(
    Conv::msToFramesRounded(10, 44100) == 441
      && Conv::msToFramesCeil(10, 44100) == 441,
    "441 frames are exact: ceil must not add a frame");
  // 1 ms at 44100 Hz = 44.1 frames
  GOAssert(Conv::msToFramesRounded(1, 44100) == 44, "44.1 rounds to 44");
  GOAssert(Conv::msToFramesCeil(1, 44100) == 45, "44.1 rounds up to 45");
  // 1 ms at 1500 Hz = 1.5 frames: halves go up
  GOAssert(Conv::msToFramesRounded(1, 1500) == 2, "1.5 rounds half up");
  GOAssert(Conv::msToFramesRounded(1, 1499) == 1, "1.499 rounds to 1");
  GOAssert(
    Conv::msToFramesRounded(0, 48000) == 0
      && Conv::msToFramesCeil(0, 48000) == 0,
    "zero duration is zero frames");
  GOAssert(
    Conv::msToFramesRounded(441000, 192000) == 84672000,
    "no overflow for the longest period at a high sample rate");
}

void GOTestSoundConversions::TestFramesToMs() {
  using Conv = GOSoundConversions;

  GOAssert(Conv::framesToMs(48000, 48000) == 1000, "one second");
  GOAssert(Conv::framesToMs(47, 48000) == 0, "0.979 ms rounds down to 0");
  GOAssert(Conv::framesToMs(48, 48000) == 1, "exactly 1 ms");
  GOAssert(Conv::framesToMs(0, 44100) == 0, "zero frames");
  GOAssert(Conv::framesToMs(84672000, 192000) == 441000, "the longest period");
}

void GOTestSoundConversions::run() {
  GO_RUN_TEST(TestGain())
  GO_RUN_TEST(TestCents())
  GO_RUN_TEST(TestMsToFrames())
  GO_RUN_TEST(TestFramesToMs())
}
