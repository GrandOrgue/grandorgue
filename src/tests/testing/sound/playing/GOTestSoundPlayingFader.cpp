/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundPlayingFader.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundPlayingFader.h"

#include "GOTestScope.h"

const std::string GOTestSoundPlayingFader::TEST_NAME
  = "GOTestSoundPlayingFader";

// the fader works on interleaved stereo
static constexpr unsigned N_CHANNELS = 2;
// the tolerance of the per-frame gains (the per-frame delta is a float
// division)
static constexpr float GAIN_TOLERANCE = 1e-5f;

/**
 * Processes a block of ones with the fader, so the output is the per-frame
 * gain.
 *
 * @param fader the fader to process
 * @param externalAmplitude the external amplitude passed to the fader
 * @param nFrames the block size
 * @return the interleaved stereo gains, nFrames * 2 items
 */
static std::vector<float> process_ones(
  GOSoundPlayingFader &fader, float externalAmplitude, unsigned nFrames) {
  std::vector<float> data(nFrames * N_CHANNELS, 1.0f);
  GOSoundBufferMutable buffer(data.data(), N_CHANNELS, nFrames);

  fader.Process(externalAmplitude, buffer);
  return data;
}

/**
 * Checks that the block of gains is the chord (the linear segment) from
 * startGain, reached at the first frame, to endGain, reached at the frame
 * after the last one; both channels.
 */
static void assert_chord(
  GOTestSoundPlayingFader &test,
  const std::vector<float> &gains,
  float startGain,
  float endGain,
  const std::string &caseName) {
  const unsigned nFrames = gains.size() / N_CHANNELS;

  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    const float expected = startGain + (endGain - startGain) * frameI / nFrames;

    for (unsigned channelI = 0; channelI < N_CHANNELS; ++channelI)
      test.GOAssert(
        std::fabs(gains[frameI * N_CHANNELS + channelI] - expected)
          <= GAIN_TOLERANCE,
        caseName + ": frame " + std::to_string(frameI) + " channel "
          + std::to_string(channelI) + " expected " + std::to_string(expected)
          + " got " + std::to_string(gains[frameI * N_CHANNELS + channelI]));
  }
}

void GOTestSoundPlayingFader::TestConstantGain() {
  GOSoundPlayingFader fader;

  fader.Setup(0.5f, 0.5f);
  GOAssert(!fader.IsSilent(), "a constant 0.5 volume must not be silent");
  // 0.5 target * 0.5 velocity * 1.0 external
  assert_chord(*this, process_ones(fader, 1.0f, 64), 0.25f, 0.25f, "block 1");
  assert_chord(*this, process_ones(fader, 1.0f, 17), 0.25f, 0.25f, "block 2");
  GOAssert(!fader.IsSilent(), "still not silent after the processing");
}

void GOTestSoundPlayingFader::TestFadeIn() {
  GOSoundPlayingFader fader;

  fader.Setup(1.0f, 1.0f, 128);
  GOAssert(fader.IsSilent(), "a fade-in starts from the silence");
  assert_chord(*this, process_ones(fader, 1.0f, 32), 0.0f, 0.25f, "block 1");
  GOAssert(!fader.IsSilent(), "not silent after the first block");
  assert_chord(*this, process_ones(fader, 1.0f, 32), 0.25f, 0.5f, "block 2");
  assert_chord(*this, process_ones(fader, 1.0f, 32), 0.5f, 0.75f, "block 3");
  // the target is reached exactly at the end of this block
  assert_chord(*this, process_ones(fader, 1.0f, 32), 0.75f, 1.0f, "block 4");
  assert_chord(*this, process_ones(fader, 1.0f, 32), 1.0f, 1.0f, "block 5");
}

void GOTestSoundPlayingFader::TestDecayToSilence() {
  GOSoundPlayingFader fader;

  fader.Setup(1.0f, 1.0f);
  fader.StartDecreasingVolume(128);
  assert_chord(*this, process_ones(fader, 1.0f, 48), 1.0f, 0.625f, "block 1");
  assert_chord(*this, process_ones(fader, 1.0f, 48), 0.625f, 0.25f, "block 2");
  GOAssert(!fader.IsSilent(), "not silent before the decay is over");
  // the decay reaches 0 in the middle of this block, but the chord goes to
  // 0 over the whole block
  assert_chord(*this, process_ones(fader, 1.0f, 48), 0.25f, 0.0f, "block 3");
  GOAssert(fader.IsSilent(), "silent after the decay is over");
  assert_chord(*this, process_ones(fader, 1.0f, 48), 0.0f, 0.0f, "block 4");
}

void GOTestSoundPlayingFader::TestExternalVolume() {
  GOSoundPlayingFader fader;

  fader.Setup(1.0f, 1.0f);
  // the first call takes the external volume as is
  assert_chord(*this, process_ones(fader, 0.5f, 64), 0.5f, 0.5f, "first");
  assert_chord(*this, process_ones(fader, 1.0f, 256), 0.5f, 1.0f, "up 256");
  assert_chord(*this, process_ones(fader, 0.5f, 1024), 1.0f, 0.5f, "down 1024");
  assert_chord(*this, process_ones(fader, 0.5f, 64), 0.5f, 0.5f, "stable");
}

void GOTestSoundPlayingFader::TestVelocityVolume() {
  GOSoundPlayingFader fader;

  fader.Setup(1.0f, 0.5f);
  GOAssert(
    fader.GetVelocityVolume() == 0.5f, "GetVelocityVolume() after Setup()");
  assert_chord(*this, process_ones(fader, 1.0f, 64), 0.5f, 0.5f, "block 1");
  fader.SetVelocityVolume(1.0f);
  GOAssert(
    fader.GetVelocityVolume() == 1.0f, "GetVelocityVolume() after Set...()");
  assert_chord(*this, process_ones(fader, 1.0f, 64), 0.5f, 1.0f, "block 2");
  assert_chord(*this, process_ones(fader, 1.0f, 64), 1.0f, 1.0f, "block 3");
}

/**
 * Makes the fader turn from a growth to a decay: a fade-in of 128 frames to 1,
 * after the first 32 frames (volume 0.25) the decay of 128 frames starts. The
 * crossing point P of the growth and the virtual decay is then 0.625,
 * 48 frames from the turn (inc = dec = 1 / 128).
 */
static void start_turned_to_decay(
  GOSoundPlayingFader &fader, unsigned nDecayFrames = 128) {
  fader.Setup(1.0f, 1.0f, 128);
  process_ones(fader, 1.0f, 32);
  fader.StartDecreasingVolume(nDecayFrames);
}

void GOTestSoundPlayingFader::TestGrowthToDecayInsideBlock() {
  GOSoundPlayingFader fader;

  // P exactly at the block end (48 frames): the growth reaches P, no decay
  start_turned_to_decay(fader);
  assert_chord(*this, process_ones(fader, 1.0f, 48), 0.25f, 0.625f, "at end");
  // the decay starts at P in the next block
  assert_chord(*this, process_ones(fader, 1.0f, 32), 0.625f, 0.375f, "after");

  // P in the middle of the block (frame 48 of 64): the real shape is
  // 0.25 -> 0.625 -> 0.5, the chord cuts the peak off
  start_turned_to_decay(fader);
  assert_chord(*this, process_ones(fader, 1.0f, 64), 0.25f, 0.5f, "middle");
  assert_chord(*this, process_ones(fader, 1.0f, 32), 0.5f, 0.25f, "after mid");

  // P at the last but one frame (frame 48 of 49), one frame of decay
  start_turned_to_decay(fader);
  assert_chord(
    *this, process_ones(fader, 1.0f, 49), 0.25f, 0.6171875f, "last frame");

  // P in the very first frames (a very fast decay): the growth gives
  // nothing, the decay takes the whole block and ends in silence
  start_turned_to_decay(fader, 1);
  assert_chord(*this, process_ones(fader, 1.0f, 32), 0.25f, 0.0f, "first");
  GOAssert(fader.IsSilent(), "the fast decay must end in silence");
}

void GOTestSoundPlayingFader::TestGrowthToDecayReachingZero() {
  GOSoundPlayingFader fader;

  // A quick decay of 8 frames from P ~ 0.29: P is reached 5 frames into the
  // next block of 64 frames, the decay is over within it and the chord goes
  // to 0 over all the 64 frames
  start_turned_to_decay(fader, 8);
  assert_chord(*this, process_ones(fader, 1.0f, 64), 0.25f, 0.0f, "to zero");
  GOAssert(fader.IsSilent(), "must be silent after the decay is over");
  assert_chord(*this, process_ones(fader, 1.0f, 64), 0.0f, 0.0f, "after");
}

void GOTestSoundPlayingFader::TestGrowthToDecayBlockCuts() {
  static constexpr unsigned BLOCK_SIZES[][2] = {{64, 0}, {32, 32}, {16, 48}};

  for (const unsigned(&blockSizes)[2] : BLOCK_SIZES) {
    GOSoundPlayingFader fader;

    start_turned_to_decay(fader);

    float maxGain = 0.0f;
    float minGain = 1.0f;

    for (unsigned blockSize : blockSizes)
      if (blockSize > 0)
        for (float gain : process_ones(fader, 1.0f, blockSize)) {
          maxGain = std::max(maxGain, gain);
          minGain = std::min(minGain, gain);
        }
    GOAssert(maxGain <= 1.0f, "the gain must not exceed the original target");
    GOAssert(minGain >= 0.0f, "the gain must not be negative");
    // 64 frames after the turn: the same state in every cut, with the peak
    // (0.625) at 48 frames passed
    assert_chord(
      *this,
      process_ones(fader, 1.0f, 32),
      0.5f,
      0.25f,
      "state after " + std::to_string(blockSizes[0]) + "+"
        + std::to_string(blockSizes[1]));
  }
}

void GOTestSoundPlayingFader::TestTargetReachedMidBlock() {
  GOSoundPlayingFader fader;

  fader.Setup(1.0f, 1.0f, 128);
  assert_chord(*this, process_ones(fader, 1.0f, 48), 0.0f, 0.375f, "block 1");
  assert_chord(*this, process_ones(fader, 1.0f, 48), 0.375f, 0.75f, "block 2");
  // The target is reached 32 frames into this block, but the chord goes to
  // the target at the block end
  assert_chord(*this, process_ones(fader, 1.0f, 48), 0.75f, 1.0f, "block 3");
  assert_chord(*this, process_ones(fader, 1.0f, 48), 1.0f, 1.0f, "block 4");
  GOAssert(!fader.IsSilent(), "must not be silent at the target");
}

void GOTestSoundPlayingFader::run() {
  GO_RUN_TEST(TestConstantGain())
  GO_RUN_TEST(TestFadeIn())
  GO_RUN_TEST(TestDecayToSilence())
  GO_RUN_TEST(TestExternalVolume())
  GO_RUN_TEST(TestVelocityVolume())
  GO_RUN_TEST(TestGrowthToDecayInsideBlock())
  GO_RUN_TEST(TestGrowthToDecayReachingZero())
  GO_RUN_TEST(TestGrowthToDecayBlockCuts())
  GO_RUN_TEST(TestTargetReachedMidBlock())
}
