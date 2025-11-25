/*
 * Copyright 2024-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestNameMap.h"

const std::string GOTestNameMap::TEST_NAME = "GOTestNameMap";

void GOTestNameMap::TestAddAndGetName() {
  GONameMap map;
  auto id1 = map.EnsureNameExists("Alice");
  auto id2 = map.EnsureNameExists("Bob");

  GOAssert(
    id1 != GONameMap::ID_NOT_IN_FILE,
    std::format(
      "id1 should not be ID_NOT_IN_FILE (name: {}, id: {})", "Alice", id1));
  GOAssert(
    id2 != GONameMap::ID_NOT_IN_FILE,
    std::format(
      "id2 should not be ID_NOT_IN_FILE (name: {}, id: {})", "Bob", id2));
  GOAssert(
    id1 != id2,
    std::format(
      "id1 and id2 should be different (Alice id: {}, Bob id: {})", id1, id2));

  GOAssert(
    map.GetNameById(id1) == "Alice",
    std::format("Name by id1 should be 'Alice' (id: {})", id1));
  GOAssert(
    map.GetNameById(id2) == "Bob",
    std::format("Name by id2 should be 'Bob' (id: {})", id2));

  GOAssert(
    map.GetIdByName("Alice") == id1,
    std::format("Id by name 'Alice' should be id1 (id: {})", id1));
  GOAssert(
    map.GetIdByName("Bob") == id2,
    std::format("Id by name 'Bob' should be id2 (id: {})", id2));

  GOAssert(
    map.GetIdByName("Charlie") == GONameMap::ID_NOT_IN_FILE,
    "Non-existing name 'Charlie' should return ID_NOT_IN_FILE");

  GOAssert(
    map.GetNameById(0).empty(), "Invalid id 0 should return empty string");
  GOAssert(
    map.GetNameById(1000).empty(),
    "Invalid id 1000 should return empty string");
}

void GOTestNameMap::TestEnsureNameExists() {
  GONameMap map;
  bool callbackCalled = false;

  auto id1 = map.EnsureNameExists("Device", [&](GONameMap::IdType id) {
    callbackCalled = true;
    GOAssert(id > 0, "Callback received invalid id for name 'Device'");
  });
  GOAssert(callbackCalled, "Callback should be called when adding 'Device'");

  callbackCalled = false;
  auto id2 = map.EnsureNameExists(
    "Device", [&](GONameMap::IdType id) { callbackCalled = true; });

  GOAssert(
    id1 == id2,
    std::format("id1 and id2 should be equal for name 'Device' (id: {})", id1));
  GOAssert(
    !callbackCalled,
    "Callback should NOT be called when name 'Device' already exists");
}

void GOTestNameMap::TestMultipleNames() {
  GONameMap map;
  for (int i = 1; i <= 10; ++i) {
    std::string name = std::format("Name{}", i);
    auto id = map.EnsureNameExists(name);

    GOAssert(
      id == i,
      std::format(
        "Id should match expected index (name: {}, id: {})", name, id));
  }
  for (int i = 1; i <= 10; ++i) {
    std::string name = std::format("Name{}", i);

    GOAssert(
      map.GetIdByName(name) == i,
      std::format(
        "GetIdByName should return correct id (name: {}, id: {})", name, i));
    GOAssert(
      map.GetNameById(i) == name,
      std::format(
        "GetNameById should return correct name (id: {}, name: {})", i, name));
  }
}

void GOTestNameMap::run() {
  TestAddAndGetName();
  TestEnsureNameExists();
  TestMultipleNames();
}
