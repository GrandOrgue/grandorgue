/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundShelfFilterProcessor.h"

#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/effects/GOSoundShelfFilterProcessor.h"
#include "sound/effects/GOSoundShelfFilterProcessorState.h"
#include "sound/processing/GOSoundProcessor.h"

const std::string GOTestSoundShelfFilterProcessor::TEST_NAME
  = "GOTestSoundShelfFilterProcessor";

static constexpr unsigned TEST_SAMPLE_RATE = 44100;
static constexpr unsigned TEST_N_FRAMES = 64;

static void fill_with_alternating_signal(GOSoundBufferPlanarMutable &buffer) {
  for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
    buffer.GetData()[itemI] = (itemI % 2 == 0) ? 0.5f : -0.5f;
}

void GOTestSoundShelfFilterProcessor::TestConstructorLeavesBothBandsNoop() {
  GOSoundShelfFilterProcessor processor;

  GOAssert(
    !processor.m_LowCoeffs.isActive,
    "a freshly constructed processor's low band must not be isActive");
  GOAssert(
    !processor.m_HighCoeffs.isActive,
    "a freshly constructed processor's high band must not be isActive");
}

void GOTestSoundShelfFilterProcessor::
  TestEnsureSetupRecomputesOnlyOnSampleRateChange() {
  GOSoundShelfFilterProcessor processor;

  processor.SetLowShelf(200, 6);
  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  const GOSoundOnePoleFilter::Coeffs afterFirstSetup = processor.m_LowCoeffs;

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);
  GOAssert(
    processor.m_LowCoeffs.b0 == afterFirstSetup.b0
      && processor.m_LowCoeffs.b1 == afterFirstSetup.b1
      && processor.m_LowCoeffs.a1 == afterFirstSetup.a1,
    "a repeated EnsureSetup() call with the same sampleRate must not "
    "recompute (and thus not disturb) an already-configured band");

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE * 2);
  GOAssert(
    processor.m_LowCoeffs.b0 != afterFirstSetup.b0
      || processor.m_LowCoeffs.b1 != afterFirstSetup.b1
      || processor.m_LowCoeffs.a1 != afterFirstSetup.a1,
    "EnsureSetup() with a changed sampleRate must recompute the "
    "coefficients");
}

void GOTestSoundShelfFilterProcessor::TestSettersAffectOnlyTheirOwnBand() {
  GOSoundShelfFilterProcessor processor;

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);
  GOAssert(
    !processor.m_LowCoeffs.isActive && !processor.m_HighCoeffs.isActive,
    "test precondition: both bands must start not isActive");

  processor.SetLowShelf(200, 6);
  GOAssert(
    processor.m_LowCoeffs.isActive,
    "SetLowShelf() must configure the low band");
  GOAssert(
    !processor.m_HighCoeffs.isActive,
    "SetLowShelf() must leave the high band untouched");

  processor.SetHighShelf(5000, -6);
  GOAssert(
    processor.m_HighCoeffs.isActive,
    "SetHighShelf() must configure the high band");
  GOAssert(
    processor.m_LowCoeffs.isActive,
    "SetHighShelf() must leave the low band's configuration untouched");
}

void GOTestSoundShelfFilterProcessor::TestCreateTypedStateSizing() {
  GOSoundShelfFilterProcessor processor;

  processor.EnsureSetup(3, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundShelfFilterProcessorState> pState
    = processor.CreateTypedState();

  GOAssert(
    pState->m_LowState.size() == 3 && pState->m_HighState.size() == 3,
    "CreateTypedState() must size both state vectors to the channel count "
    "from the most recent EnsureSetup()");
  for (float value : pState->m_LowState)
    GOAssert(value == 0.0f, "a freshly created state must be zeroed");
  for (float value : pState->m_HighState)
    GOAssert(value == 0.0f, "a freshly created state must be zeroed");
}

void GOTestSoundShelfFilterProcessor::TestProcessBothBandsNoopIsBypass() {
  GOSoundShelfFilterProcessor processor;

  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundShelfFilterProcessorState> pState
    = processor.CreateTypedState();
  GOSoundProcessor &untypedProcessor = processor;
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 2, TEST_N_FRAMES);

  fill_with_alternating_signal(buffer);
  untypedProcessor.Process(*pState, buffer);

  for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
    GOAssert(
      buffer.GetData()[itemI] == ((itemI % 2 == 0) ? 0.5f : -0.5f),
      "both bands not isActive must leave the buffer completely untouched");
}

void GOTestSoundShelfFilterProcessor::TestProcessOnlyConfiguredBandIsApplied() {
  GOSoundShelfFilterProcessor processor;

  processor.SetLowShelf(200, 6);
  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundShelfFilterProcessorState> pState
    = processor.CreateTypedState();
  GOSoundProcessor &untypedProcessor = processor;
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 2, TEST_N_FRAMES);

  fill_with_alternating_signal(buffer);
  untypedProcessor.Process(*pState, buffer);

  bool wasLowChanged = false;

  for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
    if (buffer.GetData()[itemI] != ((itemI % 2 == 0) ? 0.5f : -0.5f))
      wasLowChanged = true;
  GOAssert(
    wasLowChanged, "the configured low band must actually alter the audio");
  for (float value : pState->m_HighState)
    GOAssert(
      value == 0.0f,
      "an unconfigured (not isActive) high band's state must stay untouched");
}

void GOTestSoundShelfFilterProcessor::TestProcessAppliesBothBandsInSeries() {
  for (unsigned nChannels : {1u, 2u}) {
    GOSoundShelfFilterProcessor processor;

    processor.SetLowShelf(200, 6);
    processor.SetHighShelf(5000, -6);
    processor.EnsureSetup(nChannels, TEST_N_FRAMES, TEST_SAMPLE_RATE);

    std::unique_ptr<GOSoundShelfFilterProcessorState> pState
      = processor.CreateTypedState();
    GOSoundProcessor &untypedProcessor = processor;
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, nChannels, TEST_N_FRAMES);

    fill_with_alternating_signal(buffer);
    untypedProcessor.Process(*pState, buffer);

    bool wasChanged = false;

    for (unsigned itemI = 0; itemI < buffer.GetNItems(); itemI++)
      if (buffer.GetData()[itemI] != ((itemI % 2 == 0) ? 0.5f : -0.5f))
        wasChanged = true;
    GOAssert(
      wasChanged,
      "both bands configured must actually alter the audio, for "
      "nChannels="
        + std::to_string(nChannels));
  }
}

void GOTestSoundShelfFilterProcessor::
  TestDynamicUpdateTakesEffectImmediately() {
  GOSoundShelfFilterProcessor processor;

  processor.SetLowShelf(200, 6);
  processor.EnsureSetup(1, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundShelfFilterProcessorState> pState
    = processor.CreateTypedState();
  GOSoundProcessor &untypedProcessor = processor;
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferA, 1, TEST_N_FRAMES);

  fill_with_alternating_signal(bufferA);
  untypedProcessor.Process(*pState, bufferA);

  processor.SetLowShelf(200, 18);
  pState->Reset();

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferB, 1, TEST_N_FRAMES);

  fill_with_alternating_signal(bufferB);
  untypedProcessor.Process(*pState, bufferB);

  bool wasDifferent = false;

  for (unsigned itemI = 0; itemI < bufferA.GetNItems(); itemI++)
    if (bufferA.GetData()[itemI] != bufferB.GetData()[itemI])
      wasDifferent = true;
  GOAssert(
    wasDifferent,
    "SetLowShelf() called between two Process() calls must change the "
    "second call's output immediately");
}

void GOTestSoundShelfFilterProcessor::
  TestBandStateClearedWhenBecomingInactive() {
  GOSoundShelfFilterProcessor processor;

  processor.SetLowShelf(200, 6);
  processor.SetHighShelf(5000, -6);
  processor.EnsureSetup(2, TEST_N_FRAMES, TEST_SAMPLE_RATE);

  std::unique_ptr<GOSoundShelfFilterProcessorState> pState
    = processor.CreateTypedState();
  GOSoundProcessor &untypedProcessor = processor;
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(activeBuffer, 2, TEST_N_FRAMES);

  fill_with_alternating_signal(activeBuffer);
  untypedProcessor.Process(*pState, activeBuffer);

  bool hasLowMemory = false;
  bool hasHighMemory = false;

  for (float value : pState->m_LowState)
    hasLowMemory = hasLowMemory || value != 0.0f;
  for (float value : pState->m_HighState)
    hasHighMemory = hasHighMemory || value != 0.0f;
  GOAssert(
    hasLowMemory && hasHighMemory,
    "test precondition: active bands must carry non-zero state");

  // only the low band becomes inactive
  processor.SetLowShelf(200, 0);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(lowOffBuffer, 2, TEST_N_FRAMES);

  fill_with_alternating_signal(lowOffBuffer);
  untypedProcessor.Process(*pState, lowOffBuffer);
  for (float value : pState->m_LowState)
    GOAssert(value == 0.0f, "an inactive low band's state must be cleared");
  GOAssert(!pState->m_WasLowActive, "the low band must be marked inactive");
  GOAssert(pState->m_WasHighActive, "the high band must stay marked active");

  hasHighMemory = false;
  for (float value : pState->m_HighState)
    hasHighMemory = hasHighMemory || value != 0.0f;
  GOAssert(
    hasHighMemory, "the still active high band's state must be preserved");

  // both bands inactive
  processor.SetHighShelf(5000, 0);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bothOffBuffer, 2, TEST_N_FRAMES);

  fill_with_alternating_signal(bothOffBuffer);
  untypedProcessor.Process(*pState, bothOffBuffer);
  for (unsigned itemI = 0; itemI < bothOffBuffer.GetNItems(); itemI++)
    GOAssert(
      bothOffBuffer.GetData()[itemI] == ((itemI % 2 == 0) ? 0.5f : -0.5f),
      "inactive bands must leave the audio untouched");
  for (float value : pState->m_HighState)
    GOAssert(value == 0.0f, "an inactive high band's state must be cleared");

  // re-activate: silence in must give silence out
  processor.SetLowShelf(200, 6);
  processor.SetHighShelf(5000, -6);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(silentBuffer, 2, TEST_N_FRAMES);

  silentBuffer.FillWithSilence();
  untypedProcessor.Process(*pState, silentBuffer);
  for (unsigned itemI = 0; itemI < silentBuffer.GetNItems(); itemI++)
    GOAssert(
      silentBuffer.GetData()[itemI] == 0.0f,
      "silence must stay silent after a band is re-activated");
}

void GOTestSoundShelfFilterProcessor::run() {
  TestConstructorLeavesBothBandsNoop();
  TestEnsureSetupRecomputesOnlyOnSampleRateChange();
  TestSettersAffectOnlyTheirOwnBand();
  TestCreateTypedStateSizing();
  TestProcessBothBandsNoopIsBypass();
  TestProcessOnlyConfiguredBandIsApplied();
  TestProcessAppliesBothBandsInSeries();
  TestDynamicUpdateTakesEffectImmediately();
  TestBandStateClearedWhenBecomingInactive();
}
