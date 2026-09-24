/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundOutputTask.h"

#include <format>
#include <vector>

#include "sound/buffer/GOSoundBufferMutableMono.h"
#include "sound/reverb/GOSoundReverb.h"
#include "sound/tasks/GOSoundBufferTaskBase.h"
#include "sound/tasks/GOSoundOutputTask.h"

#include "GOTestScope.h"

const std::string GOTestSoundOutputTask::TEST_NAME = "GOTestSoundOutputTask";

static constexpr unsigned N_CHANNELS = 2;
static constexpr unsigned N_SAMPLES_PER_BUFFER = 8;

// 10 mono float32 samples, 0.1, 0.2, ..., 1.0, at 44100 Hz - the same
// fixture GOTestSoundReverb and GOTestSoundReverbProcessor use.
static const std::string TEST_IR_WAV_PATH
  = GO_TEST_RESOURCES_DIR "/sound/reverb/test-ir.wav";
static constexpr unsigned TEST_IR_SAMPLE_RATE = 44100;

// zita-convolver's Convproc::configure() requires quantum (the reverb's
// nSamplesPerBuffer) to be a power of two >= MINQUANT (16), which
// N_SAMPLES_PER_BUFFER (8) is not - Setup() would silently fail and clear
// the engine. Reverb-specific tests use their own buffer size instead,
// matching GOTestSoundReverbProcessor's TEST_N_FRAMES.
static constexpr unsigned REVERB_N_SAMPLES_PER_BUFFER = 64;

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

// Same as StubBufferTask, but sized for the reverb-specific tests (see
// REVERB_N_SAMPLES_PER_BUFFER above).
class ReverbStubBufferTask : public GOSoundBufferTaskBase {
public:
  ReverbStubBufferTask()
    : GOSoundBufferTaskBase(
      PRIORITY_AUDIOGROUP, false, N_CHANNELS, REVERB_N_SAMPLES_PER_BUFFER) {}

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

// Reads one sample of the given output channel.
float getChannelSample(
  GOSoundBufferTaskBase &task, unsigned channelI, unsigned frameI) {
  return task.GetChannelBuffer(channelI).GetData()[frameI];
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
    getChannelSample(output, 0, 0) == 0.25f,
    "channel 0 should equal the corresponding input channel");
  GOAssert(
    getChannelSample(output, 1, 0) == -0.5f,
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
    getChannelSample(output, 0, 0) == 1.0f,
    "values above 1.0 must be clamped to 1.0, independently per channel");
  GOAssert(
    getChannelSample(output, 1, 0) == -1.0f,
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
    getChannelSample(output, 1, 0) == 1.0f,
    "output channel 1 should pick up input channel 0 when routed to it");

  output.NewRound();
  fillChannel(input, 0, 0.0f);
  output.Run();

  GOAssert(
    getChannelSample(output, 1, 0) == 0.0f,
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

// A unit impulse near the end of channel 0's buffer, everything else
// silent: the 10-sample IR's tail spills past the buffer boundary into the
// next round.
static constexpr unsigned IMPULSE_FRAME = REVERB_N_SAMPLES_PER_BUFFER - 4;

// Configures output's reverb with the fixture IR shared by this file's
// reverb tests, and routes input into it.
static void setup_reverb_output(
  GOSoundBufferTaskBase &input, GOSoundOutputTask &output) {
  const GOSoundReverb::ReverbConfig config = {
    .isEnabled = true,
    .isDirect = false,
    .channel = 1,
    .startOffset = 0,
    .len = 0,
    .delay = 0,
    .gain = 1.0f,
    .file = TEST_IR_WAV_PATH,
  };

  output.SetupReverb(config, REVERB_N_SAMPLES_PER_BUFFER, TEST_IR_SAMPLE_RATE);
  output.SetOutputs({&input});
}

// Fires the impulse round and returns with the IR tail still entirely
// unplayed, pending inside the convolution engine.
static void fire_impulse_round(
  ReverbStubBufferTask &input, GOSoundOutputTask &output) {
  fillChannel(input, 0, 0.0f);
  fillChannel(input, 1, 0.0f);
  input.GetChannelBuffer(0).GetData()[IMPULSE_FRAME] = 1.0f;
  output.Run();
  input.GetChannelBuffer(0).GetData()[IMPULSE_FRAME] = 0.0f;
}

void GOTestSoundOutputTask::TestDiscardContentResetsReverbTail() {
  // Control instance, never discarded: establishes that, left alone, the
  // pending tail from the impulse round does bleed into a later round with
  // silent input - so the fixture actually exercises the regression this
  // test is meant to catch.
  ReverbStubBufferTask controlInput;
  GOSoundOutputTask control(
    N_CHANNELS, makeIdentityScaleFactors(), REVERB_N_SAMPLES_PER_BUFFER);

  setup_reverb_output(controlInput, control);
  fire_impulse_round(controlInput, control);

  bool wasTailObserved = false;

  for (unsigned roundI = 0; roundI < 8 && !wasTailObserved; roundI++) {
    control.NewRound();
    control.Run();
    for (unsigned frameI = 0;
         frameI < REVERB_N_SAMPLES_PER_BUFFER && !wasTailObserved;
         frameI++)
      wasTailObserved = getChannelSample(control, 0, frameI) != 0.0f;
  }

  GOAssert(
    wasTailObserved,
    "sanity check: the IR tail must bleed into a later round with silent "
    "input");

  // Test instance: DiscardContent() runs right after the impulse round,
  // before any further Run() call has a chance to play out (and thereby
  // drain) the pending tail on its own - so the rounds checked below can
  // only stay silent because DiscardContent() actually reset the
  // convolution engine's state, not because the tail had already finished
  // playing out naturally.
  ReverbStubBufferTask testInput;
  GOSoundOutputTask test(
    N_CHANNELS, makeIdentityScaleFactors(), REVERB_N_SAMPLES_PER_BUFFER);

  setup_reverb_output(testInput, test);
  fire_impulse_round(testInput, test);
  test.DiscardContent();

  bool wasTailObservedAfterDiscard = false;

  for (unsigned roundI = 0; roundI < 8; roundI++) {
    test.NewRound();
    test.Run();
    for (unsigned frameI = 0; frameI < REVERB_N_SAMPLES_PER_BUFFER; frameI++)
      if (getChannelSample(test, 0, frameI) != 0.0f)
        wasTailObservedAfterDiscard = true;
  }

  GOAssert(
    !wasTailObservedAfterDiscard,
    "DiscardContent() must reset the reverb engine so a leftover "
    "convolution tail does not bleed into rounds run after the task is "
    "reused without another SetupReverb() call");
  GOAssert(
    test.GetMeterInfo()[0] == 0.0f,
    "DiscardContent() must also reset the meter, as before");
}

void GOTestSoundOutputTask::
  TestIsEmptyStaysFalseAfterMeterResetWhileReverbActive() {
  ReverbStubBufferTask input;

  GOSoundOutputTask output(
    N_CHANNELS, makeIdentityScaleFactors(), REVERB_N_SAMPLES_PER_BUFFER);
  const GOSoundReverb::ReverbConfig config = {
    .isEnabled = true,
    .isDirect = false,
    .channel = 1,
    .startOffset = 0,
    .len = 0,
    .delay = 0,
    .gain = 1.0f,
    .file = TEST_IR_WAV_PATH,
  };

  output.SetupReverb(config, REVERB_N_SAMPLES_PER_BUFFER, TEST_IR_SAMPLE_RATE);
  output.SetOutputs({&input});

  GOAssert(
    output.IsEmpty(),
    "sanity check: a freshly set-up task must start IsEmpty()");

  fillChannel(input, 0, 0.0f);
  fillChannel(input, 1, 0.0f);
  input.GetChannelBuffer(0).GetData()[0] = 1.0f;
  output.Run();
  // Without this, GOSoundTaskBase::IsEmpty() would still read false from
  // the completed round alone (see
  // TestIsEmptyStaysFalseAfterSilentRoundUntilNewRound), masking a broken
  // m_Reverb->HasContent() check below. NewRound() only resets that base
  // round state - GOSoundOutputTask has no DoNewRound() override, so the
  // reverb tail and meter are untouched.
  output.NewRound();

  // ResetMeterInfo() alone stands in for GOSoundOrganEngine::NextPeriod(),
  // which calls it every period regardless of deregistration - it must not
  // be enough to make IsEmpty() report true while the reverb engine may
  // still hold an undecayed tail.
  output.ResetMeterInfo();

  GOAssert(
    !output.IsEmpty(),
    "IsEmpty() must stay false after a mere meter reset while the reverb "
    "engine has run and may still hold a tail - only DiscardContent() may "
    "clear that");

  output.DiscardContent();

  GOAssert(
    output.IsEmpty(),
    "DiscardContent() must make IsEmpty() true again by actually "
    "resetting the reverb engine");
}

void GOTestSoundOutputTask::run() {
  GO_RUN_TEST(TestIsEmptyStaysFalseAfterSilentRoundUntilNewRound())
  GO_RUN_TEST(TestDiscardContentAfterRunMakesTaskEmpty())
  GO_RUN_TEST(TestIdentityMixPassesThroughUnclamped())
  GO_RUN_TEST(TestClampsOutOfRangeValuesPerChannel())
  GO_RUN_TEST(TestMeterInfoTracksPeakPerChannelAndResets())
  GO_RUN_TEST(TestZeroScaleFactorExcludesChannel())
  GO_RUN_TEST(TestIdentityMixPreservesPerFrameLayout())
  GO_RUN_TEST(TestDiscardContentResetsReverbTail())
  GO_RUN_TEST(TestIsEmptyStaysFalseAfterMeterResetWhileReverbActive())
}
