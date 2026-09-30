/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDWINDCHESTGROUPTASK_H
#define GOTESTSOUNDWINDCHESTGROUPTASK_H

#include <string>

#include "GOTest.h"

/**
 * Exercises the real GOSoundWindchestGroupTask, not just the generic
 * cooperative-protocol double (GOSoundCooperativeTaskTestImpl). Every
 * sampler these tests Add() has p_WindchestTask == nullptr, which makes
 * ProcessList() skip GOSoundSamplerPlayer::ProcessSampler() entirely (see
 * GOSoundWindchestGroupTask.cpp) - so the tests exercise the real
 * claim/participate/merge/broadcast round protocol and sampler-list
 * draining without needing a real GOSoundProvider or decoded audio.
 */
class GOTestSoundWindchestGroupTask : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A freshly constructed task is not done and has nothing queued. */
  void TestInitialState();

  /** Add() makes GetCost() reflect both active and release samplers;
   * DiscardContent() clears them back to zero. */
  void TestAddAndDiscardContentTrackCost();

  /** IsEmpty() is true right after construction, false while either the
   * active or the release list holds a sampler, and true again after
   * DiscardContent(). */
  void TestIsEmptyTracksSamplerLists();

  /** Run() with nothing queued still completes the round with exactly one
   * participant. */
  void TestRunWithNoSamplersReachesDone();

  /** CompleteRound() finishes the round synchronously. */
  void TestCompleteRoundFinishesSynchronously();

  /** NewRound() lets the task run a fresh, independent round. */
  void TestNewRoundAllowsFreshRound();

  /** WaitAndDiscardContent() returns promptly once the round is done and
   * clears any queued content. */
  void TestWaitAndDiscardContentCompletes();

  /** Many threads racing Run() on a task with real queued samplers must
   * reach done without deadlock or crash, repeated across many rounds. */
  void TestConcurrentRunWithQueuedSamplersReachesDone();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDWINDCHESTGROUPTASK_H */
