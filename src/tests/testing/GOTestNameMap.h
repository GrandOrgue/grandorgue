/*
 * Copyright 2024-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTNAMEMAP_H
#define GOTESTNAMEMAP_H

#include "GOTest.h"

class GOTestNameMap : public GOTest {

private:
  static const std::string TEST_NAME;

  void TestAddAndGetName();
  void TestEnsureNameExists();
  void TestMultipleNames();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTNAMEMAP_H */
