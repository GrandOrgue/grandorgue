/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferPlanarManaged.h"

#include "sound/buffer/GOSoundBufferPlanarManaged.h"

#include "GOTestScope.h"

const std::string GOTestSoundBufferPlanarManaged::TEST_NAME
  = "GOTestSoundBufferPlanarManaged";

void GOTestSoundBufferPlanarManaged::TestDefaultConstructor() {
  TestManagedDefaultConstructor<GOSoundBufferPlanarManaged>(
    "PlanarManaged DefaultConstructor");
}

void GOTestSoundBufferPlanarManaged::TestConstructorWithDimensions() {
  TestManagedConstructorWithDimensions<GOSoundBufferPlanarManaged>(
    "PlanarManaged ConstructorWithDimensions");
}

void GOTestSoundBufferPlanarManaged::TestCopyConstructorFromView() {
  TestManagedCopyConstructorFromView<
    GOSoundBufferPlanarManaged,
    GOSoundBufferPlanar>("PlanarManaged CopyConstructorFromView");
}

void GOTestSoundBufferPlanarManaged::TestCopyConstructorFromManaged() {
  TestManagedCopyConstructorFromManaged<GOSoundBufferPlanarManaged>(
    "PlanarManaged CopyConstructorFromManaged");
}

void GOTestSoundBufferPlanarManaged::TestMoveConstructor() {
  TestManagedMoveConstructor<GOSoundBufferPlanarManaged>(
    "PlanarManaged MoveConstructor");
}

void GOTestSoundBufferPlanarManaged::TestCopyAssignmentFromView() {
  TestManagedCopyAssignmentFromView<
    GOSoundBufferPlanarManaged,
    GOSoundBufferPlanar>("PlanarManaged CopyAssignmentFromView");
}

void GOTestSoundBufferPlanarManaged::TestCopyAssignmentFromManaged() {
  TestManagedCopyAssignmentFromManaged<GOSoundBufferPlanarManaged>(
    "PlanarManaged CopyAssignmentFromManaged");
}

void GOTestSoundBufferPlanarManaged::TestMoveAssignment() {
  TestManagedMoveAssignment<GOSoundBufferPlanarManaged>(
    "PlanarManaged MoveAssignment");
}

void GOTestSoundBufferPlanarManaged::TestResize() {
  TestManagedResize<GOSoundBufferPlanarManaged>("PlanarManaged Resize");
}

void GOTestSoundBufferPlanarManaged::TestResizeNoReallocation() {
  TestManagedResizeNoReallocation<GOSoundBufferPlanarManaged>(
    "PlanarManaged ResizeNoReallocation");
}

void GOTestSoundBufferPlanarManaged::TestSwap() {
  TestManagedSwap<GOSoundBufferPlanarManaged>("PlanarManaged Swap");
}

void GOTestSoundBufferPlanarManaged::run() {
  GO_RUN_TEST(TestDefaultConstructor())
  GO_RUN_TEST(TestConstructorWithDimensions())
  GO_RUN_TEST(TestCopyConstructorFromView())
  GO_RUN_TEST(TestCopyConstructorFromManaged())
  GO_RUN_TEST(TestMoveConstructor())
  GO_RUN_TEST(TestCopyAssignmentFromView())
  GO_RUN_TEST(TestCopyAssignmentFromManaged())
  GO_RUN_TEST(TestMoveAssignment())
  GO_RUN_TEST(TestResize())
  GO_RUN_TEST(TestResizeNoReallocation())
  GO_RUN_TEST(TestSwap())
}
