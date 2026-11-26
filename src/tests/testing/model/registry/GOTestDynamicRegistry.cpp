/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestDynamicRegistry.h"

#include <vector>

#include "model/registry/GODynamicRegistry.h"

#include "GOTestScope.h"

GOTestDynamicRegistry::~GOTestDynamicRegistry() {}

std::string GOTestDynamicRegistry::GetName() { return name; }

void GOTestDynamicRegistry::run() {
  GO_RUN_TEST(TestAddDeduplicates())
  GO_RUN_TEST(TestRemoveKeepsOrder())
  GO_RUN_TEST(TestAddAfterRemoveAppends())
}

void GOTestDynamicRegistry::TestAddDeduplicates() {
  int objects[3];
  GODynamicRegistry<int> registry;

  registry.Add(&objects[0]);
  registry.Add(&objects[1]);
  registry.Add(&objects[0]);
  registry.Add(&objects[2]);
  GOAssert(
    registry.AsVector()
      == std::vector<int *>{&objects[0], &objects[1], &objects[2]},
    "A repeated Add should keep the object once at the first position");
}

void GOTestDynamicRegistry::TestRemoveKeepsOrder() {
  int objects[3];
  GODynamicRegistry<int> registry;

  for (int &object : objects)
    registry.Add(&object);
  registry.Remove(&objects[1]);
  GOAssert(
    registry.AsVector() == std::vector<int *>{&objects[0], &objects[2]},
    "Remove should keep the order of the other objects");

  registry.Remove(&objects[1]);
  registry.Remove(nullptr);
  GOAssert(
    registry.AsVector().size() == 2,
    "Removing an unregistered object should do nothing");
}

void GOTestDynamicRegistry::TestAddAfterRemoveAppends() {
  int objects[3];
  GODynamicRegistry<int> registry;

  for (int &object : objects)
    registry.Add(&object);
  registry.Remove(&objects[0]);
  registry.Add(&objects[0]);
  GOAssert(
    registry.AsVector()
      == std::vector<int *>{&objects[1], &objects[2], &objects[0]},
    "An object added after Remove should go to the end");
}
