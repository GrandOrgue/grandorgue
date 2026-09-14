/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDTREMULANTTASK_H
#define GOTESTSOUNDTREMULANTTASK_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundTremulantTask::IsStateful() - regression coverage for the
 * Codex finding on PR #2620 "Avoid dirtying rounds for no-op stateful task
 * runs": unlike GOSoundWindchestTask/GOSoundTouchTask, whose IsStateful() is
 * an unconditional constant, GOSoundTremulantTask's answer must track
 * whether it currently has a sampler queued - the exact condition that
 * decides whether its next Run() would actually process one.
 */
class GOTestSoundTremulantTask : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A freshly constructed task, with nothing ever Add()ed, is not
   * stateful. */
  void TestFreshTaskIsNotStateful();

  /** A queued sampler makes the task stateful, before Run() ever processes
   * it - GetNextTask() must see this at dispatch time. */
  void TestQueuedSamplerMakesTaskStateful();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDTREMULANTTASK_H */
