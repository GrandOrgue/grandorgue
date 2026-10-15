/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundWindchestGroupTask.h"

#include <memory>
#include <thread>
#include <vector>

#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/playing/GOSoundSampler.h"
#include "sound/processing/GOSoundProcessingChain.h"
#include "sound/processing/GOSoundProcessorState.h"
#include "sound/processing/GOSoundProcessorTyped.h"
#include "sound/tasks/GOSoundWindchestGroupTask.h"
#include "sound/tasks/GOSoundWindchestTask.h"

#include "../processing/GOSoundProcessingTestImpls.h"
#include "GOSoundWindchestGroupTestFixture.h"

namespace {

// Ignores buffer content entirely and writes the number of Process() calls
// seen so far (since the last Reset()) into every sample. Unlike
// GOOnePoleProcessor/GOAddConstProcessor (whose output is a pure function
// of the buffer content), this lets TestDiscardContentResetsChainState
// observe whether the chain state was actually reset, even though the
// buffer these tests' rounds merge is always silent (every dummy sampler
// has p_WindchestTask == nullptr, so ProcessList() never touches it - see
// the class comment in the .h).
class GOCallCountState : public GOSoundProcessorState {
private:
  float m_count = 0.0f;

public:
  void Reset() override { m_count = 0.0f; }
  float Increment() { return ++m_count; }
};

class GOCallCountProcessor : public GOSoundProcessorTyped<GOCallCountState> {
public:
  void EnsureSetup(unsigned, unsigned, unsigned) override {}

protected:
  std::unique_ptr<GOCallCountState> CreateTypedState() const override {
    return std::make_unique<GOCallCountState>();
  }

  void Process(GOCallCountState &state, GOSoundBufferPlanarMutable &buffer)
    const override {
    const float count = state.Increment();

    for (unsigned nItems = buffer.GetNItems(), itemI = 0; itemI < nItems;
         itemI++)
      buffer.GetData()[itemI] = count;
  }
};

// A single GOCallCountProcessor chain, built fresh per call.
std::unique_ptr<GOSoundProcessingChain> build_call_count_chain() {
  auto pChain = std::make_unique<GOSoundProcessingChain>();

  pChain->AddProcessor(std::make_unique<GOCallCountProcessor>());
  return pChain;
}

// (x + 2) * 3, in that order - lets TestRunExecutesChainInOrder prove
// processor order, not just presence.
std::unique_ptr<GOSoundProcessingChain> build_add_then_scale_chain() {
  auto pChain = std::make_unique<GOSoundProcessingChain>();

  pChain->AddProcessor(std::make_unique<GOAddConstProcessor>(2.0f));
  pChain->AddProcessor(std::make_unique<GOScaleProcessor>(3.0f));
  return pChain;
}

// x * 3 + 2, in that order - the reverse of build_add_then_scale_chain(),
// giving a different result from the same silent input (2 vs 6).
std::unique_ptr<GOSoundProcessingChain> build_scale_then_add_chain() {
  auto pChain = std::make_unique<GOSoundProcessingChain>();

  pChain->AddProcessor(std::make_unique<GOScaleProcessor>(3.0f));
  pChain->AddProcessor(std::make_unique<GOAddConstProcessor>(2.0f));
  return pChain;
}

} // namespace

const std::string GOTestSoundWindchestGroupTask::TEST_NAME
  = "GOTestSoundWindchestGroupTask";

static constexpr unsigned N_SAMPLES_PER_BUFFER = 64;
static constexpr unsigned N_STRESS_ITERATIONS = 50;
static constexpr unsigned N_STRESS_THREADS = 8;
static constexpr unsigned N_STRESS_SAMPLERS = 100;

// value-initialised: p_WindchestTask stays null, which is what makes it safe
// to Add() without a real GOSoundProvider - see the class comment in the .h
static GOSoundSampler make_dummy_sampler(bool isRelease) {
  GOSoundSampler sampler{};

  sampler.is_release = isRelease;
  return sampler;
}

// Starts nThreads running body concurrently and joins all of them
static void run_on_threads(unsigned nThreads, std::function<void()> body) {
  std::vector<std::thread> threads;

  for (unsigned threadI = 0; threadI < nThreads; threadI++)
    threads.emplace_back(body);
  for (std::thread &thread : threads)
    thread.join();
}

void GOTestSoundWindchestGroupTask::TestInitialState() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  GOAssert(!task.IsDone(), "a fresh task must not be done");
  GOAssert(task.GetCost() == 0, "a fresh task must have nothing queued");
}

void GOTestSoundWindchestGroupTask::TestAddAndDiscardContentTrackCost() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);
  GOSoundSampler activeSampler = make_dummy_sampler(false);
  GOSoundSampler releaseSampler = make_dummy_sampler(true);

  task.Add(&activeSampler);
  task.Add(&releaseSampler);
  GOAssert(
    task.GetCost() == 2,
    "GetCost() must count both active and release samplers");

  task.DiscardContent();
  GOAssert(task.GetCost() == 0, "DiscardContent() must clear queued samplers");
}

void GOTestSoundWindchestGroupTask::TestIsEmptyTracksSamplerLists() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  GOAssert(task.IsEmpty(), "a freshly constructed task must be empty");

  GOSoundSampler activeSampler = make_dummy_sampler(false);

  task.Add(&activeSampler);
  GOAssert(!task.IsEmpty(), "a task with a queued active sampler is not empty");

  task.DiscardContent();
  GOAssert(task.IsEmpty(), "DiscardContent() must clear the active sampler");

  GOSoundSampler releaseSampler = make_dummy_sampler(true);

  task.Add(&releaseSampler);
  GOAssert(
    !task.IsEmpty(), "a task with a queued release sampler is not empty");

  task.DiscardContent();
  GOAssert(task.IsEmpty(), "DiscardContent() must clear the release sampler");
}

void GOTestSoundWindchestGroupTask::TestRunWithNoSamplersReachesDone() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  task.Run();
  GOAssert(
    task.IsDone(), "Run() must complete the round even with nothing queued");
}

void GOTestSoundWindchestGroupTask::TestCompleteRoundFinishesSynchronously() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  task.CompleteRound();
  GOAssert(
    task.IsDone(), "CompleteRound() must not return before the round is done");
}

void GOTestSoundWindchestGroupTask::TestNewRoundAllowsFreshRound() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  task.Run();
  task.NewRound();
  GOAssert(!task.IsDone(), "NewRound() must reset the round state");

  task.Run();
  GOAssert(task.IsDone(), "the task must be able to run a fresh round");
}

void GOTestSoundWindchestGroupTask::TestWaitAndDiscardContentCompletes() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);
  GOSoundSampler sampler = make_dummy_sampler(false);

  task.Add(&sampler);
  task.Run();
  task.WaitAndDiscardContent();

  GOAssert(
    task.GetCost() == 0,
    "WaitAndDiscardContent() must return promptly once the round is done "
    "and clear queued content");
}

void GOTestSoundWindchestGroupTask::
  TestConcurrentRunWithQueuedSamplersReachesDone() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  for (unsigned iterI = 0; iterI < N_STRESS_ITERATIONS; iterI++) {
    std::vector<GOSoundSampler> samplers;

    samplers.reserve(N_STRESS_SAMPLERS);
    for (unsigned samplerI = 0; samplerI < N_STRESS_SAMPLERS; samplerI++)
      samplers.push_back(make_dummy_sampler(samplerI % 3 == 0));
    for (GOSoundSampler &sampler : samplers)
      task.Add(&sampler);

    run_on_threads(N_STRESS_THREADS, [&task]() { task.Run(); });

    GOAssert(
      task.IsDone(),
      std::string("iteration ") + std::to_string(iterI)
        + ": the round must end up done under concurrent Run()");

    task.NewRound();
  }
}

void GOTestSoundWindchestGroupTask::TestRunExecutesChainInOrder() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask addThenScaleTask(
    fixture.roundCounter,
    fixture.player,
    fixture.BuildWindchestTask(build_add_then_scale_chain()),
    N_SAMPLES_PER_BUFFER);
  GOSoundWindchestGroupTask scaleThenAddTask(
    fixture.roundCounter,
    fixture.player,
    fixture.BuildWindchestTask(build_scale_then_add_chain()),
    N_SAMPLES_PER_BUFFER);

  addThenScaleTask.Run();
  scaleThenAddTask.Run();

  GOAssert(
    addThenScaleTask.GetData()[0] == 6.0f,
    "OnMixed() must run (x + 2) * 3 on the merged (silent) buffer");
  GOAssert(
    scaleThenAddTask.GetData()[0] == 2.0f,
    "OnMixed() must run x * 3 + 2 in processor order, not (x + 2) * 3");
}

void GOTestSoundWindchestGroupTask::
  TestRunForcesWindchestTaskEvenWithNoSamplers() {
  GOSoundWindchestGroupTestFixture fixture;
  auto pChain = std::make_unique<GOSoundProcessingChain>();

  pChain->AddProcessor(std::make_unique<GOAddConstProcessor>(1.0f));

  GOSoundWindchestTask &windchestTask
    = fixture.BuildWindchestTask(std::move(pChain));
  GOSoundWindchestGroupTask task(
    fixture.roundCounter, fixture.player, windchestTask, N_SAMPLES_PER_BUFFER);

  GOAssert(
    !windchestTask.IsDone(),
    "a freshly built windchest task must not be done yet");

  // no samplers Add()ed: this round is empty on purpose
  task.Run();

  GOAssert(
    windchestTask.IsDone(),
    "OnMixed() must force the owning windchest task's own round to "
    "completion even when this round had no samplers, so a stateful "
    "processor still sees current parameters");
  GOAssert(
    task.GetData()[0] == 1.0f,
    "the chain must still run on a zero-sampler round (a single "
    "GOAddConstProcessor(1.0f) turns silence into 1.0f)");
}

void GOTestSoundWindchestGroupTask::TestDiscardContentResetsChainState() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTask task(
    fixture.roundCounter,
    fixture.player,
    fixture.BuildWindchestTask(build_call_count_chain()),
    N_SAMPLES_PER_BUFFER);

  task.Run();
  GOAssert(task.GetData()[0] == 1.0f, "the first round must see call count 1");

  task.NewRound();
  task.Run();
  GOAssert(
    task.GetData()[0] == 2.0f,
    "without a reset, the chain state must keep accumulating across rounds");

  task.DiscardContent();
  task.NewRound();
  task.Run();
  GOAssert(
    task.GetData()[0] == 1.0f,
    "DiscardContent() must reset the chain state, not just the sampler "
    "lists - the call count must restart from 1");
}

void GOTestSoundWindchestGroupTask::run() {
  TestInitialState();
  TestAddAndDiscardContentTrackCost();
  TestIsEmptyTracksSamplerLists();
  TestRunWithNoSamplersReachesDone();
  TestCompleteRoundFinishesSynchronously();
  TestNewRoundAllowsFreshRound();
  TestWaitAndDiscardContentCompletes();
  TestConcurrentRunWithQueuedSamplersReachesDone();
  TestRunExecutesChainInOrder();
  TestRunForcesWindchestTaskEvenWithNoSamplers();
  TestDiscardContentResetsChainState();
}
