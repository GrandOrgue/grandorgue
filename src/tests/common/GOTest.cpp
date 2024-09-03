/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include "GOTest.h"
#include "GOOrganController.h"
#include "GOTestCollection.h"
#include "config/GOConfig.h"
#include <cstdio>
#include <iostream>

GOTest::GOTest() {
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
  GOConfig settings(this->GetName());
  this->controller = new GOOrganController(settings);
  this->controller->InitOrganDirectory(this->organ_directory);
  return true;
}

bool GOCommonControllerTest::tearDown() {
  // This initialize a new GOOrganController object that will be destroyed
  // during test teardown().
  this->controller = nullptr;
  unlink(this->organ_directory);
  return true;
}