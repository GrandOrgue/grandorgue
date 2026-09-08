/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSCHEDULER_H
#define GOTESTSCHEDULER_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOScheduler's discard-on-deregister contract: a task registered
 * via Add() must arrive IsEmpty() (Add() asserts it), and a task taken out
 * via Clear() or Remove() must have DiscardContent() called on it exactly
 * once, so it is safe to Add() back without accumulated content leaking
 * across the deregistration.
 */
class GOTestScheduler : public GOTest {
private:
  static const std::string TEST_NAME;

  /** Clear() must call DiscardContent() on every registered task exactly
   * once, leaving each task IsEmpty(). */
  void TestClearDiscardsContentOnEveryTask();

  /** Remove() must call DiscardContent() on the removed task, symmetrically
   * with Add()'s IsEmpty() assertion, so a task removed individually - not
   * only via Clear() - is safe to Add() back. */
  void TestRemoveDiscardsContent();

  /** Remove(nullptr) must be a no-op, matching Add(nullptr). */
  void TestRemoveNullptrIsNoop();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSCHEDULER_H */
