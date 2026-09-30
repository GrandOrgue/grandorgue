/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundWindchestGroupTask.h"

#include <memory>
#include <thread>
#include <vector>

#include "sound/playing/GOSoundSampler.h"
#include "sound/tasks/GOSoundWindchestGroupTask.h"

#include "GOSoundWindchestGroupTestFixture.h"

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

void GOTestSoundWindchestGroupTask::run() {
  TestInitialState();
  TestAddAndDiscardContentTrackCost();
  TestIsEmptyTracksSamplerLists();
  TestRunWithNoSamplersReachesDone();
  TestCompleteRoundFinishesSynchronously();
  TestNewRoundAllowsFreshRound();
  TestWaitAndDiscardContentCompletes();
  TestConcurrentRunWithQueuedSamplersReachesDone();
}
