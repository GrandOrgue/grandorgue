/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEST_H
#define GOTEST_H

#include <optional>

#include "GOOrganController.h"
#include "GOTestUtils.h"

#include "config/GOConfig.h"

class GOTest : public GOTestUtils {
  /*
      We define here all methods that will be called to run tests properly.
          - A setup
          - The run itself
          - A tear down method
  */

public:
  enum Category { FUNCTIONAL, PERF };

private:
  std::string name = "GOTest";
  Category m_Category;

public:
  GOTest(Category category = FUNCTIONAL);
  virtual ~GOTest();
  virtual bool setUp();
  virtual void run();
  virtual bool tearDown();
  virtual std::string GetName() { return name; }
  Category GetCategory() const { return m_Category; }
};

class GOCommonControllerTest : public GOTest {
private:
  // Owns the GOConfig that controller's GOOrganController::m_config
  // reference points to. GOConfig has no default constructor and needs
  // GetName(), which only resolves to the derived test's actual name once
  // the derived constructor has run - so this is constructed in setUp(),
  // not in the constructor's initializer list, via std::optional's
  // deferred-construction (hence the mp_ prefix, matching this codebase's
  // convention for members that are absent until explicitly constructed).
  // Must outlive controller: tearDown() deletes controller before
  // resetting mp_config (see GOTest.cpp), since ~GOOrganController() may
  // still read m_config.
  std::optional<GOConfig> mp_config;

public:
  GOCommonControllerTest();
  char *organ_directory;
  GOOrganController *controller;
  bool setUp();
  virtual bool tearDown();
};

#endif