/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundOutputTask.h"

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

void GOTestSoundOutputTask::run() {
  GO_RUN_TEST(TestIsEmptyStaysFalseAfterSilentRoundUntilNewRound())
  GO_RUN_TEST(TestDiscardContentAfterRunMakesTaskEmpty())
}
