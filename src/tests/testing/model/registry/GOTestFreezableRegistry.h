/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTFREEZABLEREGISTRY_H
#define GOTESTFREEZABLEREGISTRY_H

#include <string>

#include "GOTest.h"

/**
 * Tests GOFreezableRegistry. It is a friend of the registry to check its
 * internal state.
 */
class GOTestFreezableRegistry : public GOTest {
private:
  std::string name = "GOTestFreezableRegistry";

  /** An object registered twice is kept once at its first position. */
  void TestAddDeduplicates();

  /** Remove leaves a tombstone, which is dropped on freezing. */
  void TestRemoveLeavesTombstone();

  /** An object added after Remove goes to the end. */
  void TestAddAfterRemoveAppends();

  /** The transitions between the frozen and not frozen states. */
  void TestSetFrozen();

  /** Adding and removing many objects stays linear. */
  void TestManyObjects();

public:
  GOTestFreezableRegistry() { name = "GOTestFreezableRegistry"; }
  virtual ~GOTestFreezableRegistry();
  virtual void run();
  std::string GetName();
};

#endif /* GOTESTFREEZABLEREGISTRY_H */
