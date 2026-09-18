/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDWINDCHESTGROUPTASKGRID_H
#define GOTESTSOUNDWINDCHESTGROUPTASKGRID_H

#include <string>

#include "GOTest.h"

class GOTestSoundWindchestGroupTaskGrid : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A freshly resized grid has no constructed tasks: HasWindchestGroupTask()
   * is false for every cell and GetInputsForGroup() returns an empty vector
   * for every group. */
  void TestResizeStartsEmpty();

  /** HasWindchestGroupTask() is false before BuildWindchestGroupTask() and
   * true afterwards, and BuildWindchestGroupTask() returns a non-null task.
   */
  void TestHasBecomesTrueAfterBuild();

  /** BuildWindchestGroupTask() for two different pairs constructs two
   * distinct task instances. */
  void TestBuildDoesNotOverShare();

  /** GetWindchestGroupTask() returns the same instance
   * BuildWindchestGroupTask() constructed. */
  void TestGetWindchestGroupTaskReturnsBuiltInstance();

  /** GetInputsForGroup() returns exactly the non-null tasks for a given
   * audio group, across every windchest index, in windchest-index order. */
  void TestGetInputsForGroupCollectsColumn();

  /** ForEachTask() visits every constructed task exactly once, skipping
   * unused cells. */
  void TestForEachTaskVisitsEveryConstructedTask();

  /** Clear() destroys every owned task; ForEachTask() then visits nothing.
   */
  void TestClearEmptiesTheGrid();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDWINDCHESTGROUPTASKGRID_H */
