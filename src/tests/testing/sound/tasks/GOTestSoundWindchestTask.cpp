/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundWindchestTask.h"

#include <memory>

#include "config/GOConfig.h"
#include "model/GOOrganModel.h"
#include "sound/GOSoundOrganEngine.h"
#include "sound/processing/GOSoundProcessingChain.h"
#include "sound/tasks/GOSoundWindchestTask.h"

#include "GOMemoryPool.h"

#include "../processing/GOSoundProcessingTestImpls.h"

const std::string GOTestSoundWindchestTask::TEST_NAME
  = "GOTestSoundWindchestTask";

namespace {

// A minimal, standalone engine for GOSoundWindchestTask's r_engine
// reference. Safe without BuildEngine()/StartEngine(): none of GOMemoryPool,
// GOConfig, or GOOrganModel do any I/O in their constructors - see
// GOSoundWindchestGroupTestFixture's class comment for the fuller
// justification of this pattern.
struct GOWindchestTaskTestEngine {
  GOMemoryPool memoryPool;
  GOConfig config;
  GOOrganModel organModel;
  GOSoundOrganEngine engine;

  GOWindchestTaskTestEngine()
    : config("GOTestSoundWindchestTask", ""),
      organModel(config),
      engine(organModel, memoryPool) {}
};

} // namespace

void GOTestSoundWindchestTask::TestGetChainNeverNull() {
  GOWindchestTaskTestEngine testEngine;
  GOSoundWindchestTask emptyChainTask(
    testEngine.engine, nullptr, std::make_unique<GOSoundProcessingChain>());
  auto pChainWithProcessor = std::make_unique<GOSoundProcessingChain>();

  pChainWithProcessor->AddProcessor(
    std::make_unique<GOAddConstProcessor>(1.0f));

  GOSoundWindchestTask nonEmptyChainTask(
    testEngine.engine, nullptr, std::move(pChainWithProcessor));

  GOAssert(
    emptyChainTask.GetChain().IsEmpty(),
    "GetChain() must never be null and must be empty for an empty chain");
  GOAssert(
    !nonEmptyChainTask.GetChain().IsEmpty(),
    "GetChain() must reflect a chain with a processor");
}

void GOTestSoundWindchestTask::TestChainReflectsConstructedChain() {
  GOWindchestTaskTestEngine testEngine;
  auto pChain = std::make_unique<GOSoundProcessingChain>();

  pChain->AddProcessor(std::make_unique<GOAddConstProcessor>(1.0f));
  pChain->AddProcessor(std::make_unique<GOAddConstProcessor>(1.0f));
  pChain->AddProcessor(std::make_unique<GOAddConstProcessor>(1.0f));

  GOSoundWindchestTask task(testEngine.engine, nullptr, std::move(pChain));

  GOAssert(
    task.GetChain().GetNProcessors() == 3,
    "GetChain() must expose exactly the processors the task was constructed "
    "with");
}

void GOTestSoundWindchestTask::TestDoRunCallsEnsureParametersUpToDate() {
  GOWindchestTaskTestEngine testEngine;
  std::unique_ptr<GOSoundProcessingChain> pChain
    = std::make_unique<GOSoundProcessingChain>();
  auto pOwnedProcessor = std::make_unique<GOAddConstProcessor>(0.0f);
  GOAddConstProcessor *pProcessor = pOwnedProcessor.get();

  pChain->AddProcessor(std::move(pOwnedProcessor));

  auto pOwnedMapper = std::make_unique<GOCountingMapper>(*pProcessor);
  GOCountingMapper *pMapper = pOwnedMapper.get();

  pChain->AddMapper(std::move(pOwnedMapper));
  pChain->EnsureSetup(2, 1, 44100);

  GOSoundWindchestTask task(testEngine.engine, nullptr, std::move(pChain));

  GOAssert(
    pMapper->GetNCalls() == 0,
    "a freshly built chain must not have had its parameters mapped yet");

  task.GetAmplitude();
  GOAssert(
    pMapper->GetNCalls() == 1,
    "GetAmplitude() must drive DoRun(), which must call the chain's "
    "EnsureParametersUpToDate() exactly once");

  task.NewRound();
  task.GetAmplitude();
  GOAssert(
    pMapper->GetNCalls() == 2,
    "a fresh round must call EnsureParametersUpToDate() again");
}

void GOTestSoundWindchestTask::run() {
  TestGetChainNeverNull();
  TestChainReflectsConstructedChain();
  TestDoRunCallsEnsureParametersUpToDate();
}
