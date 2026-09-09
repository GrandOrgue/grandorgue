/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDOUTPUTTASK_H
#define GOTESTSOUNDOUTPUTTASK_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundOutputTask's IsEmpty()/DiscardContent() contract.
 */
class GOTestSoundOutputTask : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A completed round that produced only silence, with reverb disabled,
   * must still make IsEmpty() false until NewRound() runs - meter and
   * reverb are silent, but the task is not GOSoundTaskBase::IsEmpty()
   * (not RUN_STATE_NOT_STARTED), and a caller must not treat a
   * done-but-not-reset task as safe to Add() back. */
  void TestIsEmptyStaysFalseAfterSilentRoundUntilNewRound();

  /** DiscardContent() must leave the task IsEmpty(): besides resetting the
   * meter and reverb, it must also reset the round state (NewRound()),
   * or a task Run() once and then DiscardContent()'d would still fail
   * IsEmpty(). */
  void TestDiscardContentAfterRunMakesTaskEmpty();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDOUTPUTTASK_H */
