/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundProcessingChain.h"

#include <cstring>
#include <format>
#include <vector>

#include "GOSoundProcessingTestImpls.h"
#include "sound/buffer/GOSoundBufferPlanarManaged.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/processing/GOSoundProcessingChain.h"
#include "sound/processing/GOSoundProcessingChainState.h"

const std::string GOTestSoundProcessingChain::TEST_NAME
  = "GOTestSoundProcessingChain";

void GOTestSoundProcessingChain::TestEmptyChainLeavesBufferUntouched() {
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 2, 8);

  fillWithSequential(buffer, 1.0f);

  GOSoundBufferPlanarManaged snapshot(buffer);
  GOSoundProcessingChain chain;

  GOAssert(chain.IsEmpty(), "a chain with no processors should be empty");

  chain.EnsureSetup(2, 8, 44100);

  std::unique_ptr<GOSoundProcessingChainState> pState = chain.CreateState();

  GOAssert(
    pState->GetNStates() == 0,
    "an empty chain's state should have no processor states");

  pState->Reset(); // must not crash on an empty state
  pState->Process(buffer);

  GOAssert(
    std::memcmp(buffer.GetData(), snapshot.GetData(), buffer.GetNBytes()) == 0,
    "an empty chain must leave the buffer untouched");
}

void GOTestSoundProcessingChain::TestProcessorsRunInOrder() {
  std::vector<int> log;
  GOSoundProcessingChain loggingChain;

  loggingChain.AddProcessor(
    std::make_unique<GOAddConstProcessor>(0.0f, &log, 1));
  loggingChain.AddProcessor(
    std::make_unique<GOAddConstProcessor>(0.0f, &log, 2));
  loggingChain.AddProcessor(
    std::make_unique<GOAddConstProcessor>(0.0f, &log, 3));
  loggingChain.EnsureSetup(1, 1, 44100);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(logBuffer, 1, 1);

  logBuffer.FillWithSilence();

  std::unique_ptr<GOSoundProcessingChainState> pLoggingState
    = loggingChain.CreateState();

  pLoggingState->Process(logBuffer);

  GOAssert(
    log.size() == 3 && log[0] == 1 && log[1] == 2 && log[2] == 3,
    "processors should run in the order they were added");

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(arithBuffer, 1, 1);

  arithBuffer.FillWithSilence();

  GOSoundProcessingChain addThenScale;

  addThenScale.AddProcessor(std::make_unique<GOAddConstProcessor>(1.0f));
  addThenScale.AddProcessor(std::make_unique<GOScaleProcessor>(2.0f));
  addThenScale.EnsureSetup(1, 1, 44100);
  addThenScale.CreateState()->Process(arithBuffer);

  GOAssert(
    arithBuffer.GetData()[0] == 2.0f,
    std::format(
      "add(+1)-then-scale(x2) on 0 should give 2.0 (got: {})",
      arithBuffer.GetData()[0]));

  arithBuffer.FillWithSilence();

  GOSoundProcessingChain scaleThenAdd;

  scaleThenAdd.AddProcessor(std::make_unique<GOScaleProcessor>(2.0f));
  scaleThenAdd.AddProcessor(std::make_unique<GOAddConstProcessor>(1.0f));
  scaleThenAdd.EnsureSetup(1, 1, 44100);
  scaleThenAdd.CreateState()->Process(arithBuffer);

  GOAssert(
    arithBuffer.GetData()[0] == 1.0f,
    std::format(
      "scale(x2)-then-add(+1) on 0 should give 1.0 (got: {})",
      arithBuffer.GetData()[0]));
}

void GOTestSoundProcessingChain::TestEnsureSetupReachesEveryProcessor() {
  GOSoundProcessingChain chain;
  GOAddConstProcessor *pProcessors[3];

  for (int processorI = 0; processorI < 3; processorI++) {
    std::unique_ptr<GOAddConstProcessor> pProcessor
      = std::make_unique<GOAddConstProcessor>(0.0f);

    pProcessors[processorI] = pProcessor.get();
    chain.AddProcessor(std::move(pProcessor));
  }

  chain.EnsureSetup(2, 64, 44100);

  for (int processorI = 0; processorI < 3; processorI++) {
    GOAddConstProcessor *pProcessor = pProcessors[processorI];

    GOAssert(
      pProcessor->GetNSetups() == 1,
      std::format(
        "processor {} should have received exactly one EnsureSetup() call",
        processorI));
    GOAssert(
      pProcessor->GetLastNChannels() == 2 && pProcessor->GetLastNFrames() == 64
        && pProcessor->GetLastSampleRate() == 44100,
      std::format(
        "processor {} should have received the exact EnsureSetup() arguments",
        processorI));
  }
}

void GOTestSoundProcessingChain::TestResetReachesEveryState() {
  GOSoundProcessingChain chain;

  for (int processorI = 0; processorI < 3; processorI++)
    chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f));
  chain.EnsureSetup(1, 1, 44100);

  std::unique_ptr<GOSoundProcessingChainState> pState = chain.CreateState();

  pState->Reset();

  // Being a friend of GOSoundProcessingChainState, inspect each state
  // directly: the public interface alone (GetNStates()) cannot confirm
  // Reset() individually reached every one of them.
  for (const std::unique_ptr<GOSoundProcessorState> &pRawState :
       pState->m_states) {
    GOCountingProcessorState *pCountingState
      = dynamic_cast<GOCountingProcessorState *>(pRawState.get());

    GOAssert(
      pCountingState && pCountingState->GetNResets() == 1,
      "each processor state should have received exactly one Reset() call");
  }
}

void GOTestSoundProcessingChain::
  TestEnsureParametersUpToDateReachesEveryMapper() {
  GOSoundProcessingChain chain;
  std::unique_ptr<GOAddConstProcessor> pTargetProcessorOwner
    = std::make_unique<GOAddConstProcessor>(1.0f);
  GOAddConstProcessor *pTargetProcessor = pTargetProcessorOwner.get();

  chain.AddProcessor(std::move(pTargetProcessorOwner));
  chain.AddMapper(std::make_unique<GOCountingMapper>(*pTargetProcessor));
  chain.AddMapper(
    std::make_unique<GOCountingMapper>(*pTargetProcessor, true, 5.0f));

  GOAssert(
    chain.m_mappers.size() == 2,
    "chain.m_mappers should have exactly the 2 added mappers");

  chain.EnsureSetup(1, 1, 44100);
  chain.EnsureParametersUpToDate();

  for (const std::unique_ptr<GOSoundProcessingPrmMapper> &pRawMapper :
       chain.m_mappers) {
    GOCountingMapper *pCountingMapper
      = dynamic_cast<GOCountingMapper *>(pRawMapper.get());

    GOAssert(
      pCountingMapper && pCountingMapper->GetNCalls() == 1,
      "each mapper should have received exactly one EnsureParametersUpToDate() "
      "call");
  }

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 1, 1);

  buffer.FillWithSilence();
  chain.CreateState()->Process(buffer);

  GOAssert(
    buffer.GetData()[0] == 5.0f,
    std::format(
      "the mapper's new constant should have taken effect (got: {})",
      buffer.GetData()[0]));
}

void GOTestSoundProcessingChain::TestInsertProcessorInsertsAtPosition() {
  std::vector<int> log;
  GOSoundProcessingChain chain;

  // Build phase: 1, 3, then insert 2 in between -> expected order 1, 2, 3.
  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f, &log, 1));
  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f, &log, 3));
  chain.InsertProcessor(
    1, std::make_unique<GOAddConstProcessor>(0.0f, &log, 2));

  GOAssert(
    chain.GetNProcessors() == 3,
    "InsertProcessor() should grow the processor count by one");

  chain.EnsureSetup(1, 1, 44100);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 1, 1);

  buffer.FillWithSilence();
  chain.CreateState()->Process(buffer);

  GOAssert(
    log.size() == 3 && log[0] == 1 && log[1] == 2 && log[2] == 3,
    "InsertProcessor() should splice the new processor into the requested "
    "position in the signal path");
}

void GOTestSoundProcessingChain::
  TestRemoveProcessorReturnsOwnershipAndShiftsOrder() {
  std::vector<int> log;
  GOSoundProcessingChain chain;

  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f, &log, 1));

  std::unique_ptr<GOAddConstProcessor> pMiddleOwner
    = std::make_unique<GOAddConstProcessor>(0.0f, &log, 2);
  GOAddConstProcessor *pMiddleProcessor = pMiddleOwner.get();

  chain.AddProcessor(std::move(pMiddleOwner));
  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f, &log, 3));

  std::unique_ptr<GOSoundProcessor> pRemoved = chain.RemoveProcessor(1);

  GOAssert(
    pRemoved.get() == pMiddleProcessor,
    "RemoveProcessor() should return ownership of the exact object removed");
  GOAssert(
    chain.GetNProcessors() == 2,
    "RemoveProcessor() should shrink the processor count by one");

  chain.EnsureSetup(1, 1, 44100);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 1, 1);

  buffer.FillWithSilence();
  chain.CreateState()->Process(buffer);

  GOAssert(
    log.size() == 2 && log[0] == 1 && log[1] == 3,
    "the removed processor's id should no longer appear in the Process() "
    "order, and the later processor should shift into its place");
}

void GOTestSoundProcessingChain::TestRemoveMapperAllowsRemovingItsProcessor() {
  GOSoundProcessingChain chain;
  std::unique_ptr<GOAddConstProcessor> pProcessorOwner
    = std::make_unique<GOAddConstProcessor>(0.0f);
  GOAddConstProcessor *pProcessor = pProcessorOwner.get();

  chain.AddProcessor(std::move(pProcessorOwner));

  std::unique_ptr<GOSoundProcessingPrmMapper> pMapperOwner
    = std::make_unique<GOCountingMapper>(*pProcessor);
  GOSoundProcessingPrmMapper *pMapper = pMapperOwner.get();

  chain.AddMapper(std::move(pMapperOwner));

  // Removing the processor while its mapper is still installed must be
  // refused (asserted); remove the mapper first.
  std::unique_ptr<GOSoundProcessingPrmMapper> pRemovedMapper
    = chain.RemoveMapper(*pMapper);

  GOAssert(
    pRemovedMapper.get() == pMapper,
    "RemoveMapper() should return ownership of the exact mapper removed");
  GOAssert(chain.m_mappers.empty(), "RemoveMapper() should remove the mapper");

  std::unique_ptr<GOSoundProcessor> pRemovedProcessor
    = chain.RemoveProcessor(0);

  GOAssert(
    pRemovedProcessor.get() == pProcessor,
    "RemoveProcessor() should now succeed once no mapper references the "
    "processor any more");
}

void GOTestSoundProcessingChain::TestClearChainRemovesProcessorsAndMappers() {
  GOSoundProcessingChain chain;
  std::unique_ptr<GOAddConstProcessor> pProcessorOwner1
    = std::make_unique<GOAddConstProcessor>(0.0f);
  std::unique_ptr<GOAddConstProcessor> pProcessorOwner2
    = std::make_unique<GOAddConstProcessor>(0.0f);
  GOAddConstProcessor *pProcessor1 = pProcessorOwner1.get();
  GOAddConstProcessor *pProcessor2 = pProcessorOwner2.get();

  chain.AddProcessor(std::move(pProcessorOwner1));
  chain.AddProcessor(std::move(pProcessorOwner2));
  chain.AddMapper(std::make_unique<GOCountingMapper>(*pProcessor1));
  chain.AddMapper(std::make_unique<GOCountingMapper>(*pProcessor2));

  chain.ClearChain();

  GOAssert(
    chain.IsEmpty() && chain.GetNProcessors() == 0,
    "ClearChain() should remove every processor");
  GOAssert(chain.m_mappers.empty(), "ClearChain() should remove every mapper");

  // The chain must still be fully usable afterward, exactly as if freshly
  // constructed.
  std::vector<int> log;

  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f, &log, 42));
  chain.EnsureSetup(1, 1, 44100);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 1, 1);

  buffer.FillWithSilence();
  chain.CreateState()->Process(buffer);

  GOAssert(
    log.size() == 1 && log[0] == 42,
    "a chain rebuilt after ClearChain() should process normally");
}

void GOTestSoundProcessingChain::TestIsValidReflectsChainState() {
  GOSoundProcessingChain chain;

  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f));
  chain.EnsureSetup(1, 1, 44100);

  std::unique_ptr<GOSoundProcessingChainState> pState = chain.CreateState();

  GOAssert(
    pState->IsValid(),
    "a freshly created state must be valid against its chain");

  // P1: reconfiguring the format under a live state must invalidate it.
  chain.EnsureSetup(2, 1, 44100);

  GOAssert(
    !pState->IsValid(),
    "a state must become invalid once the chain's format changes under it");

  // Cleanup() wipes the format out from under any still-alive state too.
  GOSoundProcessingChain otherChain;

  otherChain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f));
  otherChain.EnsureSetup(1, 1, 44100);

  std::unique_ptr<GOSoundProcessingChainState> pOtherState
    = otherChain.CreateState();

  GOAssert(pOtherState->IsValid(), "a freshly created state must be valid");

  otherChain.Cleanup();

  GOAssert(
    !pOtherState->IsValid(),
    "a state must become invalid once its chain is Cleanup()'d");
}

void GOTestSoundProcessingChain::TestEnsureSetupNoOpDoesNotInvalidateState() {
  GOSoundProcessingChain chain;

  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f));
  chain.EnsureSetup(1, 1, 44100);

  std::unique_ptr<GOSoundProcessingChainState> pState = chain.CreateState();

  GOAssert(
    pState->IsValid(),
    "a freshly created state must be valid against its chain");

  // Repeating EnsureSetup() with the exact same arguments must not disturb
  // a live state: per GOSoundProcessor's own contract, a same-arguments
  // call is a cheap no-op, so nothing the state depends on has changed.
  chain.EnsureSetup(1, 1, 44100);

  GOAssert(
    pState->IsValid(),
    "a same-arguments EnsureSetup() call must not invalidate a live state");
}

void GOTestSoundProcessingChain::TestChainReusableAfterAllStatesDestroyed() {
  std::vector<int> firstLog;
  GOSoundProcessingChain chain;

  chain.AddProcessor(std::make_unique<GOAddConstProcessor>(0.0f, &firstLog, 1));
  chain.InsertProcessor(
    0, std::make_unique<GOAddConstProcessor>(0.0f, &firstLog, 0));
  chain.EnsureSetup(1, 1, 44100);

  {
    std::unique_ptr<GOSoundProcessingChainState> pState = chain.CreateState();

    GOAssert(pState->IsValid(), "the initial state must be valid");

    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 1, 1);

    buffer.FillWithSilence();
    pState->Process(buffer);

    GOAssert(
      firstLog.size() == 2 && firstLog[0] == 0 && firstLog[1] == 1,
      "the initial chain should process in the built order");
  } // pState destroyed here: no live state remains.

  // Full teardown and rebuild with a different topology and format.
  chain.Cleanup();
  chain.ClearChain();

  std::vector<int> secondLog;

  chain.AddProcessor(
    std::make_unique<GOAddConstProcessor>(0.0f, &secondLog, 7));
  chain.AddProcessor(
    std::make_unique<GOAddConstProcessor>(0.0f, &secondLog, 8));
  chain.RemoveProcessor(0);
  chain.EnsureSetup(2, 4, 48000);

  std::unique_ptr<GOSoundProcessingChainState> pFreshState
    = chain.CreateState();

  GOAssert(
    pFreshState->IsValid(),
    "a state created after a full rebuild must be valid");

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, 2, 4);

  buffer.FillWithSilence();
  pFreshState->Process(buffer);

  GOAssert(
    secondLog.size() == 1 && secondLog[0] == 8,
    "the rebuilt chain should reflect the new topology (only processor 8 "
    "remains) rather than any trace of the old one");
}

void GOTestSoundProcessingChain::run() {
  TestEmptyChainLeavesBufferUntouched();
  TestProcessorsRunInOrder();
  TestEnsureSetupReachesEveryProcessor();
  TestResetReachesEveryState();
  TestEnsureParametersUpToDateReachesEveryMapper();
  TestInsertProcessorInsertsAtPosition();
  TestRemoveProcessorReturnsOwnershipAndShiftsOrder();
  TestRemoveMapperAllowsRemovingItsProcessor();
  TestClearChainRemovesProcessorsAndMappers();
  TestIsValidReflectsChainState();
  TestEnsureSetupNoOpDoesNotInvalidateState();
  TestChainReusableAfterAllStatesDestroyed();
}
