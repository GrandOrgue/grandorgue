/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include "GOTest.h"
#include "GOOrganController.h"
#include "GOTestCollection.h"
#include "config/GOConfig.h"
#include <cstdio>
#include <iostream>

GOTest::GOTest(Category category) : m_Category(category) {
  // This is the magic to auto register tests in TestCollection
  GOTestCollection::Instance()->add_test(this);
}
GOTest::~GOTest() {}

bool GOTest::setUp() {
  std::cout << "==================== " << this->GetName()
            << " - BEGIN ====================";
  return true;
}

void GOTest::run() {}

bool GOTest::tearDown() { return true; }

/* Class that initialize a new controller at each test setUp run */

GOCommonControllerTest::GOCommonControllerTest() {}

bool GOCommonControllerTest::setUp() {
  // This initialize a new GOOrganController object that will be destroyed
  // during test teardown().

  // Make organ temporary directory
  GOTest::setUp();
  char path[] = ".";
  this->organ_directory = mkdtemp(path);
  mp_config.emplace(GetName(), "");
  this->controller = new GOOrganController(*mp_config);
  this->controller->InitOrganDirectory(this->organ_directory);
  return true;
}

bool GOCommonControllerTest::tearDown() {
  // Delete controller before resetting mp_config: ~GOOrganController() may
  // still read m_config, so mp_config must outlive it.
  delete this->controller;
  this->controller = nullptr;
  mp_config.reset();
  unlink(this->organ_directory);
  return true;
}