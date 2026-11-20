/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestFreezableRegistry.h"

#include <chrono>
#include <vector>

#include "model/registry/GOFreezableRegistry.h"

#include "GOTestScope.h"

GOTestFreezableRegistry::~GOTestFreezableRegistry() {}

std::string GOTestFreezableRegistry::GetName() { return name; }

void GOTestFreezableRegistry::run() {
  GO_RUN_TEST(TestAddDeduplicates())
  GO_RUN_TEST(TestRemoveLeavesTombstone())
  GO_RUN_TEST(TestAddAfterRemoveAppends())
  GO_RUN_TEST(TestSetFrozen())
  GO_RUN_TEST(TestManyObjects())
}

void GOTestFreezableRegistry::TestAddDeduplicates() {
  int objects[3];
  GOFreezableRegistry<int> registry;

  registry.Add(&objects[0]);
  registry.Add(&objects[1]);
  registry.Add(&objects[0]);
  registry.Add(&objects[2]);
  registry.SetFrozen(true);
  GOAssert(
    registry.AsVector()
      == std::vector<int *>{&objects[0], &objects[1], &objects[2]},
    "A repeated Add should keep the object once at the first position");
}

void GOTestFreezableRegistry::TestRemoveLeavesTombstone() {
  int objects[3];
  GOFreezableRegistry<int> registry;

  for (int &object : objects)
    registry.Add(&object);
  registry.Remove(&objects[1]);
  GOAssert(
    registry.mp_objects
      == std::vector<int *>{&objects[0], nullptr, &objects[2]},
    "Remove should leave a tombstone in place of the object");
  GOAssert(
    registry.m_IndexByObject.size() == 2,
    "The index should not contain the removed object");

  registry.Remove(&objects[1]);
  registry.Remove(nullptr);
  GOAssert(
    registry.mp_objects.size() == 3 && registry.m_IndexByObject.size() == 2,
    "Removing an unregistered object should do nothing");

  registry.SetFrozen(true);
  GOAssert(
    registry.AsVector() == std::vector<int *>{&objects[0], &objects[2]},
    "Freezing should drop the tombstones keeping the order");
}

void GOTestFreezableRegistry::TestAddAfterRemoveAppends() {
  int objects[3];
  GOFreezableRegistry<int> registry;

  for (int &object : objects)
    registry.Add(&object);
  registry.Remove(&objects[0]);
  registry.Add(&objects[0]);
  GOAssert(
    registry.mp_objects
      == std::vector<int *>{nullptr, &objects[1], &objects[2], &objects[0]},
    "An object added after Remove should go to the end");
  registry.SetFrozen(true);
  GOAssert(
    registry.AsVector()
      == std::vector<int *>{&objects[1], &objects[2], &objects[0]},
    "The tombstone should be dropped on freezing");
}

void GOTestFreezableRegistry::TestSetFrozen() {
  int objects[3];
  GOFreezableRegistry<int> registry;

  GOAssert(!registry.IsFrozen(), "A new registry should not be frozen");
  for (int &object : objects)
    registry.Add(&object);
  registry.Remove(&objects[1]);

  registry.SetFrozen(true);
  GOAssert(registry.IsFrozen(), "The registry should be frozen");
  GOAssert(registry.m_IndexByObject.empty(), "Freezing should clear the index");

  registry.SetFrozen(true);
  GOAssert(
    registry.AsVector().size() == 2, "A repeated freezing should do nothing");

  registry.SetFrozen(false);
  GOAssert(!registry.IsFrozen(), "The registry should be unfrozen");
  GOAssert(
    registry.m_IndexByObject.size() == 2
      && registry.m_IndexByObject.at(&objects[0]) == 0
      && registry.m_IndexByObject.at(&objects[2]) == 1,
    "Unfreezing should rebuild the index with the positions");

  registry.Add(&objects[1]);
  registry.Remove(&objects[0]);
  registry.SetFrozen(true);
  GOAssert(
    registry.AsVector() == std::vector<int *>{&objects[2], &objects[1]},
    "Add and Remove should work after unfreezing");
}

void GOTestFreezableRegistry::TestManyObjects() {
  constexpr size_t N_OBJECTS = 100000;
  constexpr size_t STEP = 7919; // a prime, so the removal order is scattered
  std::vector<int> objects(N_OBJECTS);
  GOFreezableRegistry<int> registry;
  const auto start = std::chrono::steady_clock::now();

  for (int &object : objects)
    registry.Add(&object);
  for (size_t removedI = 0, index = 0; removedI < N_OBJECTS / 2; ++removedI) {
    registry.Remove(&objects[index]);
    index = (index + STEP) % N_OBJECTS;
  }
  registry.SetFrozen(true);
  GOAssert(
    registry.AsVector().size() == N_OBJECTS - N_OBJECTS / 2,
    "Half of the objects should remain");
  GOAssert(
    std::chrono::steady_clock::now() - start < std::chrono::seconds(5),
    "Adding and removing many objects should not be quadratic");
}
