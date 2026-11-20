/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTDYNAMICREGISTRY_H
#define GOTESTDYNAMICREGISTRY_H

#include <string>

#include "GOTest.h"

/** Tests GODynamicRegistry. */
class GOTestDynamicRegistry : public GOTest {
private:
  std::string name = "GOTestDynamicRegistry";

  /** An object registered twice is kept once at its first position. */
  void TestAddDeduplicates();

  /** Remove keeps the order of the other objects. */
  void TestRemoveKeepsOrder();

  /** An object added after Remove goes to the end. */
  void TestAddAfterRemoveAppends();

public:
  GOTestDynamicRegistry() { name = "GOTestDynamicRegistry"; }
  virtual ~GOTestDynamicRegistry();
  virtual void run();
  std::string GetName();
};

#endif /* GOTESTDYNAMICREGISTRY_H */
