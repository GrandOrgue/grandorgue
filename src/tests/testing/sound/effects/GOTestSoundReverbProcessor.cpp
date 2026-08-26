/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundReverbProcessor.h"

#include <cmath>

#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/effects/GOSoundReverbProcessor.h"
#include "sound/effects/GOSoundReverbProcessorState.h"
#include "sound/processing/GOSoundProcessor.h"

const std::string GOTestSoundReverbProcessor::TEST_NAME
  = "GOTestSoundReverbProcessor";

// resources/sound/reverb/test-ir.wav: 10 mono float32 samples, 0.1, 0.2,
// ..., 1.0, at 44100 Hz.
static const std::string TEST_IR_WAV_PATH
  = GO_TEST_RESOURCES_DIR "/sound/reverb/test-ir.wav";
static constexpr unsigned TEST_SAMPLE_RATE = 44100;
static constexpr unsigned TEST_N_FRAMES = 64;

static GOSoundReverb::ReverbConfig make_config() {
  return {
    .isEnabled = true,
    .isDirect = true,
    .channel = 1,
    .startOffset = 0,
    .len = 0,
    .delay = 0,
    .gain = 1.0f,
    .file = TEST_IR_WAV_PATH,
  };
}

static GOSoundReverb::ReverbConfig make_config_with_missing_file() {
  GOSoundReverb::ReverbConfig config = make_config();

  config.file = "/nonexistent/path/does-not-exist.wav";

  return config;
}

void GOTestSoundReverbProcessor::
  TestCreateTypedStateBuildsOneConvprocPerChannel() {
  GOSoundReverbProcessor processor(make_config());

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundReverbProcessorState> pState
    = processor.CreateTypedState();

  GOAssert(
    pState->mp_ConvprocsByChannel.size() == 2,
    "CreateTypedState() must build exactly one Convproc per channel");
  for (auto &pConvProc : pState->mp_ConvprocsByChannel)
    GOAssert(!!pConvProc, "every built Convproc must be non-null");
}

void GOTestSoundReverbProcessor::TestEnsureSetupReloadsOnFormatChange() {
  GOSoundReverbProcessor processor(make_config());

  processor.EnsureSetup(1, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundReverbProcessorState> pFirstState
    = processor.CreateTypedState();

  GOAssert(
    pFirstState->mp_ConvprocsByChannel.size() == 1,
    "the first EnsureSetup(1, ...) must produce a 1-channel state");
  pFirstState.reset();

  processor.EnsureSetup(3, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundReverbProcessorState> pSecondState
    = processor.CreateTypedState();

  GOAssert(
    pSecondState->mp_ConvprocsByChannel.size() == 3,
    "a later EnsureSetup(3, ...) must reconfigure to a 3-channel state, not "
    "keep serving the cached 1-channel format");
}

void GOTestSoundReverbProcessor::TestProcessRunsAcrossSeveralRounds() {
  GOSoundReverbProcessor processor(make_config());

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundReverbProcessorState> pState
    = processor.CreateTypedState();
  GOSoundProcessor &untypedProcessor = processor;
  bool wasChanged = false;

  for (unsigned roundI = 0; roundI < 8; roundI++) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 2, TEST_N_FRAMES);

    for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
      buffer.GetData()[itemI] = 0.5f;

    untypedProcessor.Process(*pState, buffer);

    GOAssert(
      buffer.GetNChannels() == 2 && buffer.GetNFrames() == TEST_N_FRAMES,
      "Process() must not change the buffer's shape");

    for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
      if (std::abs(buffer.GetData()[itemI] - 0.5f) > 1e-4f)
        wasChanged = true;
  }

  GOAssert(
    wasChanged,
    "the convolution reverb must actually alter the audio across several "
    "rounds of a constant input - a silent bypass (e.g. an unbuilt/no-op "
    "state slipping through) would leave every sample at the input's "
    "constant 0.5");
}

void GOTestSoundReverbProcessor::TestResetDoesNotCrash() {
  GOSoundReverbProcessor processor(make_config());

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundReverbProcessorState> pState
    = processor.CreateTypedState();

  pState->Reset();

  GOSoundProcessor &untypedProcessor = processor;
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 2, TEST_N_FRAMES);

  buffer.FillWithSilence();
  untypedProcessor.Process(*pState, buffer);
  pState->Reset();

  GOAssert(true, "Reset() before and after Process() must not crash");
}

void GOTestSoundReverbProcessor::TestMissingIrFileBuildsNoOpState() {
  GOSoundReverbProcessor processor(make_config_with_missing_file());

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundReverbProcessorState> pState
    = processor.CreateTypedState();

  GOAssert(
    pState->mp_ConvprocsByChannel.empty(),
    "a missing IR file must build a no-op state (no Convproc engines) "
    "instead of starting engines with nothing to convolve");

  GOSoundProcessor &untypedProcessor = processor;
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 2, TEST_N_FRAMES);

  for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
    buffer.GetData()[itemI] = 0.5f;
  untypedProcessor.Process(*pState, buffer);

  for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
    GOAssert(
      buffer.GetData()[itemI] == 0.5f,
      "Process() on a no-op state must be a silent bypass, leaving the "
      "buffer untouched");

  pState->Reset();
  GOAssert(true, "Reset() on a no-op state must not crash");
}

void GOTestSoundReverbProcessor::run() {
  TestCreateTypedStateBuildsOneConvprocPerChannel();
  TestEnsureSetupReloadsOnFormatChange();
  TestProcessRunsAcrossSeveralRounds();
  TestResetDoesNotCrash();
  TestMissingIrFileBuildsNoOpState();
}
