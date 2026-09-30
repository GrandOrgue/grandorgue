/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundWindchestGroupTaskGrid.h"

#include <vector>

#include "sound/GOSoundWindchestGroupTaskGrid.h"
#include "sound/tasks/GOSoundWindchestGroupTask.h"

#include "tasks/GOSoundWindchestGroupTestFixture.h"

const std::string GOTestSoundWindchestGroupTaskGrid::TEST_NAME
  = "GOTestSoundWindchestGroupTaskGrid";

static constexpr unsigned N_SAMPLES_PER_BUFFER = 64;

void GOTestSoundWindchestGroupTaskGrid::TestResizeStartsEmpty() {
  GOSoundWindchestGroupTaskGrid grid;

  grid.Resize(3, 2);
  GOAssert(
    !grid.HasWindchestGroupTask(0, 0),
    "a freshly resized grid must have no constructed tasks");
  for (unsigned groupI = 0; groupI < 2; groupI++)
    GOAssert(
      grid.GetInputsForGroup(groupI, 3).empty(),
      "a freshly resized grid must have no constructed tasks");
}

void GOTestSoundWindchestGroupTaskGrid::TestHasBecomesTrueAfterBuild() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTaskGrid grid;

  grid.Resize(2, 2);
  GOAssert(
    !grid.HasWindchestGroupTask(0, 1),
    "HasWindchestGroupTask() must be false for an unbuilt cell");

  GOSoundWindchestGroupTask *pBuilt = grid.BuildWindchestGroupTask(
    0,
    1,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  GOAssert(pBuilt, "BuildWindchestGroupTask() must return a non-null task");
  GOAssert(
    grid.HasWindchestGroupTask(0, 1),
    "HasWindchestGroupTask() must be true once the cell is built");
  GOAssert(
    &grid.GetWindchestGroupTask(0, 1) == pBuilt,
    "GetWindchestGroupTask() must return the instance "
    "BuildWindchestGroupTask() "
    "constructed");
}

void GOTestSoundWindchestGroupTaskGrid::TestBuildDoesNotOverShare() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTaskGrid grid;

  grid.Resize(2, 2);
  grid.BuildWindchestGroupTask(
    0,
    0,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);
  grid.BuildWindchestGroupTask(
    1,
    0,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(1),
    N_SAMPLES_PER_BUFFER);

  GOSoundWindchestGroupTask &atWindchest0 = grid.GetWindchestGroupTask(0, 0);
  GOSoundWindchestGroupTask &atWindchest1 = grid.GetWindchestGroupTask(1, 0);

  GOAssert(
    &atWindchest0 != &atWindchest1,
    "two different pairs must get two distinct task instances");
}

void GOTestSoundWindchestGroupTaskGrid::
  TestGetWindchestGroupTaskReturnsBuiltInstance() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTaskGrid grid;

  grid.Resize(1, 1);
  grid.BuildWindchestGroupTask(
    0,
    0,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);

  GOSoundBufferTaskBase *pFromColumn = grid.GetInputsForGroup(0, 1).at(0);
  GOSoundWindchestGroupTask &fromGet = grid.GetWindchestGroupTask(0, 0);

  GOAssert(
    pFromColumn == &fromGet,
    "GetWindchestGroupTask() must return the same instance that "
    "GetInputsForGroup() reports for the same cell");
}

void GOTestSoundWindchestGroupTaskGrid::TestGetInputsForGroupCollectsColumn() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTaskGrid grid;

  // 3 windchests x 2 groups; only (0, 0) and (2, 0) feed group 0
  grid.Resize(3, 2);
  grid.BuildWindchestGroupTask(
    0,
    0,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);
  grid.BuildWindchestGroupTask(
    2,
    0,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(2),
    N_SAMPLES_PER_BUFFER);
  grid.BuildWindchestGroupTask(
    1,
    1,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(1),
    N_SAMPLES_PER_BUFFER);

  std::vector<GOSoundBufferTaskBase *> inputs = grid.GetInputsForGroup(0, 3);

  GOAssert(
    inputs.size() == 2,
    "GetInputsForGroup() must skip the windchest that doesn't feed the "
    "requested group");
  GOAssert(
    inputs[0] == &grid.GetWindchestGroupTask(0, 0),
    "the first collected task must be the lowest-windchestIndex one");
  GOAssert(
    inputs[1] == &grid.GetWindchestGroupTask(2, 0),
    "the second collected task must be the highest-windchestIndex one");
}

void GOTestSoundWindchestGroupTaskGrid::
  TestForEachTaskVisitsEveryConstructedTask() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTaskGrid grid;

  grid.Resize(2, 2);
  grid.BuildWindchestGroupTask(
    0,
    0,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);
  grid.BuildWindchestGroupTask(
    1,
    1,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(1),
    N_SAMPLES_PER_BUFFER);

  unsigned nVisited = 0;

  grid.ForEachTask([&nVisited](GOSoundWindchestGroupTask *) { nVisited++; });

  GOAssert(
    nVisited == 2,
    "ForEachTask() must visit exactly the constructed tasks, skipping "
    "unused cells");
}

void GOTestSoundWindchestGroupTaskGrid::TestClearEmptiesTheGrid() {
  GOSoundWindchestGroupTestFixture fixture;
  GOSoundWindchestGroupTaskGrid grid;

  grid.Resize(2, 2);
  grid.BuildWindchestGroupTask(
    0,
    0,
    fixture.roundCounter,
    fixture.player,
    fixture.GetWindchestTask(0),
    N_SAMPLES_PER_BUFFER);
  grid.Clear();

  unsigned nVisited = 0;

  grid.ForEachTask([&nVisited](GOSoundWindchestGroupTask *) { nVisited++; });

  GOAssert(nVisited == 0, "Clear() must leave no task behind");
}

void GOTestSoundWindchestGroupTaskGrid::run() {
  TestResizeStartsEmpty();
  TestHasBecomesTrueAfterBuild();
  TestBuildDoesNotOverShare();
  TestGetWindchestGroupTaskReturnsBuiltInstance();
  TestGetInputsForGroupCollectsColumn();
  TestForEachTaskVisitsEveryConstructedTask();
  TestClearEmptiesTheGrid();
}
