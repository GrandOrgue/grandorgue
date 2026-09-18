/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundOutputTask.h"

#include <format>
#include <vector>

#include "sound/buffer/GOSoundBufferMutableMono.h"
#include "sound/tasks/GOSoundBufferTaskBase.h"
#include "sound/tasks/GOSoundOutputTask.h"

#include "GOTestScope.h"

const std::string GOTestSoundOutputTask::TEST_NAME = "GOTestSoundOutputTask";

static constexpr unsigned N_CHANNELS = 2;
static constexpr unsigned N_SAMPLES_PER_BUFFER = 8;

void GOTestSoundOutputTask::
  TestIsEmptyStaysFalseAfterSilentRoundUntilNewRound() {
  // No SetOutputs()/SetupReverb() call: m_OutputCount stays 0, so DoRun()
  // mixes nothing and reverb stays disabled - a plain silent round, with
  // nothing to mask a missing round-state check in IsEmpty().
  GOSoundOutputTask output(N_CHANNELS, {}, N_SAMPLES_PER_BUFFER);

  GOAssert(
    output.IsEmpty(),
    "sanity check: a freshly set-up task must start IsEmpty()");

  output.Run();

  GOAssert(
    !output.IsEmpty(),
    "a completed round - even one that produced only silence - must make "
    "IsEmpty() false: the task is done for this round "
    "(GOSoundTaskBase::IsEmpty() would say so) and not yet reset, so it is "
    "not safe to Add() back until NewRound() runs");

  output.NewRound();

  GOAssert(
    output.IsEmpty(),
    "NewRound() must make IsEmpty() true again for a silent, non-reverb "
    "task");
}

void GOTestSoundOutputTask::TestDiscardContentAfterRunMakesTaskEmpty() {
  GOSoundOutputTask output(N_CHANNELS, {}, N_SAMPLES_PER_BUFFER);

  output.Run();
  output.DiscardContent();

  GOAssert(
    output.IsEmpty(),
    "DiscardContent() must reset the round state as well as the meter and "
    "reverb, so the task is IsEmpty() afterwards regardless of whether "
    "Run() had already completed a round");
}

namespace {

// Stand-in for a real GOSoundGroupTask/GOSoundWindchestGroupTask input: lets
// the test fill fixed values directly instead of building a real windchest
// and sampler chain.
class StubBufferTask : public GOSoundBufferTaskBase {
public:
  StubBufferTask()
    : GOSoundBufferTaskBase(
      PRIORITY_AUDIOGROUP, false, N_CHANNELS, N_SAMPLES_PER_BUFFER) {}

  bool DoRun(GOSchedulerThread *) override { return true; }
  void EnsureBufferReady(bool, GOSchedulerThread * = nullptr) override {}
};

// Identity scale-factor matrix for one 2-channel input: output channel i
// takes input channel i with factor 1 and ignores the other channel.
std::vector<float> makeIdentityScaleFactors() { return {1, 0, 0, 1}; }

// Fills every frame of the given channel with a single value.
void fillChannel(GOSoundBufferTaskBase &task, unsigned channelI, float value) {
  GOSoundBufferMutableMono channel = task.GetChannelBuffer(channelI);
  const unsigned nFrames = task.GetNFrames();

  for (unsigned frameI = 0; frameI < nFrames; frameI++)
    channel.GetData()[frameI] = value;
}

// A distinct, clamp-safe value per (channel, frame): GOSoundOutputTask
// clamps every output sample to [-1, 1], so the ramp must stay inside that
// range or the clamp step (not a layout bug) would explain a mismatch.
float rampValue(unsigned channelI, unsigned frameI) {
  return 0.01f * static_cast<float>(channelI * 10 + frameI);
}

// Fills a channel with rampValue() at every frame, so a channel/frame swap
// or stride bug - the class of bug a planar-layout change risks - shows up
// as a wrong value instead of passing unnoticed the way a constant fill
// would.
void fillChannelRamp(GOSoundBufferTaskBase &task, unsigned channelI) {
  GOSoundBufferMutableMono channel = task.GetChannelBuffer(channelI);
  const unsigned nFrames = task.GetNFrames();

  for (unsigned frameI = 0; frameI < nFrames; frameI++)
    channel.GetData()[frameI] = rampValue(channelI, frameI);
}

} // namespace

void GOTestSoundOutputTask::TestIdentityMixPassesThroughUnclamped() {
  StubBufferTask input;

  fillChannel(input, 0, 0.25f);
  fillChannel(input, 1, -0.5f);

  GOSoundOutputTask output(
    N_CHANNELS, makeIdentityScaleFactors(), N_SAMPLES_PER_BUFFER);

  output.SetOutputs({&input});
  output.Run();

  GOAssert(
    output.GetChannelBuffer(0).GetData()[0] == 0.25f,
    "channel 0 should equal the corresponding input channel");
  GOAssert(
    output.GetChannelBuffer(1).GetData()[0] == -0.5f,
    "channel 1 should equal the corresponding input channel");
}

void GOTestSoundOutputTask::TestClampsOutOfRangeValuesPerChannel() {
  StubBufferTask input;

  fillChannel(input, 0, 2.0f);
  fillChannel(input, 1, -3.0f);

  GOSoundOutputTask output(
    N_CHANNELS, makeIdentityScaleFactors(), N_SAMPLES_PER_BUFFER);

  output.SetOutputs({&input});
  output.Run();

  GOAssert(
    output.GetChannelBuffer(0).GetData()[0] == 1.0f,
    "values above 1.0 must be clamped to 1.0, independently per channel");
  GOAssert(
    output.GetChannelBuffer(1).GetData()[0] == -1.0f,
    "values below -1.0 must be clamped to -1.0, independently per channel");
}

void GOTestSoundOutputTask::TestMeterInfoTracksPeakPerChannelAndResets() {
  StubBufferTask input;

  fillChannel(input, 0, 0.4f);
  fillChannel(input, 1, -0.9f);

  GOSoundOutputTask output(
    N_CHANNELS, makeIdentityScaleFactors(), N_SAMPLES_PER_BUFFER);

  output.SetOutputs({&input});
  output.Run();

  const std::vector<float> &meterAfterFirst = output.GetMeterInfo();

  GOAssert(
    meterAfterFirst[0] == 0.4f,
    "channel 0's peak must equal the absolute value seen this round");
  GOAssert(
    meterAfterFirst[1] == 0.9f,
    "channel 1's peak must equal the absolute value seen this round");

  // a smaller value must not lower the peak - meters track the maximum seen
  // since the last reset, not the latest round
  fillChannel(input, 0, 0.1f);
  fillChannel(input, 1, -0.1f);
  output.NewRound();
  output.Run();

  const std::vector<float> &meterAfterSecond = output.GetMeterInfo();

  GOAssert(
    meterAfterSecond[0] == 0.4f && meterAfterSecond[1] == 0.9f,
    "a quieter round must not lower the tracked peak before ResetMeterInfo()");

  output.ResetMeterInfo();

  const std::vector<float> &meterAfterReset = output.GetMeterInfo();

  GOAssert(
    meterAfterReset[0] == 0.0f && meterAfterReset[1] == 0.0f,
    "ResetMeterInfo() must clear every channel's peak back to 0");
}

void GOTestSoundOutputTask::TestZeroScaleFactorExcludesChannel() {
  StubBufferTask input;

  fillChannel(input, 0, 1.0f);
  fillChannel(input, 1, 1.0f);

  // route only input channel 0 into both output channels
  std::vector<float> scaleFactors = {1, 0, 1, 0};
  GOSoundOutputTask output(N_CHANNELS, scaleFactors, N_SAMPLES_PER_BUFFER);

  output.SetOutputs({&input});
  output.Run();

  GOAssert(
    output.GetChannelBuffer(1).GetData()[0] == 1.0f,
    "output channel 1 should pick up input channel 0 when routed to it");

  output.NewRound();
  fillChannel(input, 0, 0.0f);
  output.Run();

  GOAssert(
    output.GetChannelBuffer(1).GetData()[0] == 0.0f,
    "a zero scale factor for input channel 1 must exclude it from the mix");
}

void GOTestSoundOutputTask::TestIdentityMixPreservesPerFrameLayout() {
  StubBufferTask input;

  fillChannelRamp(input, 0);
  fillChannelRamp(input, 1);

  GOSoundOutputTask output(
    N_CHANNELS, makeIdentityScaleFactors(), N_SAMPLES_PER_BUFFER);

  output.SetOutputs({&input});
  output.Run();

  for (unsigned channelI = 0; channelI < N_CHANNELS; channelI++) {
    GOSoundBufferMutableMono outChannel = output.GetChannelBuffer(channelI);

    for (unsigned frameI = 0; frameI < N_SAMPLES_PER_BUFFER; frameI++)
      GOAssert(
        outChannel.GetData()[frameI] == rampValue(channelI, frameI),
        std::format(
          "channel {} frame {}: expected the input's per-frame ramp value "
          "to survive unchanged - a wrong value here means a channel/frame "
          "swap or stride bug in the planar mix",
          channelI,
          frameI));
  }
}

void GOTestSoundOutputTask::run() {
  GO_RUN_TEST(TestIsEmptyStaysFalseAfterSilentRoundUntilNewRound())
  GO_RUN_TEST(TestDiscardContentAfterRunMakesTaskEmpty())
  GO_RUN_TEST(TestIdentityMixPassesThroughUnclamped())
  GO_RUN_TEST(TestClampsOutOfRangeValuesPerChannel())
  GO_RUN_TEST(TestMeterInfoTracksPeakPerChannelAndResets())
  GO_RUN_TEST(TestZeroScaleFactorExcludesChannel())
  GO_RUN_TEST(TestIdentityMixPreservesPerFrameLayout())
}
