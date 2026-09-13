/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundProcessorTyped.h"

#include <cmath>
#include <cstring>
#include <format>
#include <vector>

#include "GOSoundProcessingTestImpls.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/processing/GOSoundProcessingChain.h"
#include "sound/processing/GOSoundProcessingChainState.h"

const std::string GOTestSoundProcessorTyped::TEST_NAME
  = "GOTestSoundProcessorTyped";

void GOTestSoundProcessorTyped::TestOneProcessorTwoIndependentStates() {
  const float alpha = 0.5f;
  const std::vector<float> sequenceA = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  const std::vector<float> sequenceB = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
  const unsigned nSamples = sequenceA.size();

  GOSoundProcessingChain chain;

  chain.AddProcessor(std::make_unique<GOOnePoleProcessor>(alpha));
  chain.EnsureSetup(1, 1, 44100);

  std::unique_ptr<GOSoundProcessingChainState> pStateA = chain.CreateState();
  std::unique_ptr<GOSoundProcessingChainState> pStateB = chain.CreateState();

  GOAssert(
    pStateA->m_states[0].get() != pStateB->m_states[0].get(),
    "the two chain states must hold two distinct processor state objects");

  std::vector<float> gotA(nSamples);
  std::vector<float> gotB(nSamples);
  float expectedPrevA = 0.0f;
  float expectedPrevB = 0.0f;

  for (unsigned sampleI = 0; sampleI < nSamples; sampleI++) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferA, 1, 1);

    bufferA.GetData()[0] = sequenceA[sampleI];
    pStateA->Process(bufferA);
    gotA[sampleI] = bufferA.GetData()[0];

    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferB, 1, 1);

    bufferB.GetData()[0] = sequenceB[sampleI];
    pStateB->Process(bufferB);
    gotB[sampleI] = bufferB.GetData()[0];

    const float expectedA
      = alpha * sequenceA[sampleI] + (1.0f - alpha) * expectedPrevA;
    const float expectedB
      = alpha * sequenceB[sampleI] + (1.0f - alpha) * expectedPrevB;

    GOAssert(
      std::abs(gotA[sampleI] - expectedA) < 1e-6f,
      std::format(
        "state A sample {}: expected {} (got: {})",
        sampleI,
        expectedA,
        gotA[sampleI]));
    GOAssert(
      std::abs(gotB[sampleI] - expectedB) < 1e-6f,
      std::format(
        "state B sample {}: expected {} (got: {})",
        sampleI,
        expectedB,
        gotB[sampleI]));

    expectedPrevA = expectedA;
    expectedPrevB = expectedB;
  }

  // Direct proof B never perturbed A: re-run sequence A alone, in a fresh
  // chain state of the same chain, and expect a bit-identical result.
  std::unique_ptr<GOSoundProcessingChainState> pSoloStateA
    = chain.CreateState();
  std::vector<float> soloA(nSamples);

  for (unsigned sampleI = 0; sampleI < nSamples; sampleI++) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 1, 1);

    buffer.GetData()[0] = sequenceA[sampleI];
    pSoloStateA->Process(buffer);
    soloA[sampleI] = buffer.GetData()[0];
  }

  GOAssert(
    std::memcmp(soloA.data(), gotA.data(), nSamples * sizeof(float)) == 0,
    "state A's output must be bit-identical whether or not state B was "
    "interleaved with it");
}

void GOTestSoundProcessorTyped::TestTypedDispatch() {
  GOOnePoleProcessor processor(0.5f);

  processor.EnsureSetup(1, 1, 44100);

  std::unique_ptr<GOSoundProcessorState> pState = processor.CreateState();

  GOAssert(
    dynamic_cast<GOOnePoleState *>(pState.get()),
    "CreateState() should return an object of the concrete typed state type");

  // Call through the untyped base entry point and confirm it reaches the
  // typed Process() override (same formula as the typed test above, with
  // alpha=0.5 and a single sample of amplitude 1.0: expected output 0.5).
  GOSoundProcessor &untypedProcessor = processor;
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 1, 1);

  buffer.GetData()[0] = 1.0f;
  untypedProcessor.Process(*pState, buffer);

  GOAssert(
    std::abs(buffer.GetData()[0] - 0.5f) < 1e-6f,
    std::format(
      "the untyped Process() call should reach the typed override (got: {})",
      buffer.GetData()[0]));
}

void GOTestSoundProcessorTyped::run() {
  TestOneProcessorTwoIndependentStates();
  TestTypedDispatch();
}
