/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundTremulantTask.h"

#include "sound/playing/GOSoundSampler.h"
#include "sound/tasks/GOSoundTremulantTask.h"

#include "GOSoundWindchestGroupTestFixture.h"
#include "GOTestScope.h"

const std::string GOTestSoundTremulantTask::TEST_NAME
  = "GOTestSoundTremulantTask";

static constexpr unsigned N_SAMPLES_PER_BUFFER = 64;

// value-initialised: safe to Add() without a real GOSoundProvider, since
// these tests never call Run()/DoRun() - only Add()'s effect on
// IsStateful() is under test here (see GOTestSoundWindchestGroupTask.cpp's
// make_dummy_sampler for the same convention)
static GOSoundSampler make_dummy_sampler() { return GOSoundSampler{}; }

void GOTestSoundTremulantTask::TestFreshTaskIsNotStateful() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundTremulantTask task(fixture.player, N_SAMPLES_PER_BUFFER);

  GOAssert(
    !task.IsStateful(),
    "a freshly constructed tremulant task, with nothing queued, must not be "
    "stateful - its next Run() would just set m_amplitude = 1 without "
    "touching any sampler");
}

void GOTestSoundTremulantTask::TestQueuedSamplerMakesTaskStateful() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundTremulantTask task(fixture.player, N_SAMPLES_PER_BUFFER);
  GOSoundSampler sampler = make_dummy_sampler();

  task.Add(&sampler);

  GOAssert(
    task.IsStateful(),
    "a tremulant task with a queued sampler must be stateful, even before "
    "Run() has processed it - GetNextTask() checks IsStateful() at dispatch "
    "time, not after the fact");
}

void GOTestSoundTremulantTask::run() {
  GO_RUN_TEST(TestFreshTaskIsNotStateful())
  GO_RUN_TEST(TestQueuedSamplerMakesTaskStateful())
}
