/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <iostream>

#include "GOTest.h"
#include "GOTestCollection.h"
#include "GOTestWindchest.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"

GOTestWindchest::GOTestWindchest() {}
GOTestWindchest::~GOTestWindchest() {}

bool GOTestWindchest::setUp() { return true; }
bool GOTestWindchest::tearDown() { return true; }

void GOTestWindchest::run() {
  char path[] = ".";
  char *dir_name = mkdtemp(path);
  GOConfig settings(wxT("Windchest Test"));
  GOOrganController *organController = new GOOrganController(settings);

  organController->InitOrganDirectory(dir_name);

  // Check the size of Windchest is correct
  unsigned w_size;
  w_size = organController->AddWindchest(new GOWindchest(*organController));
  if (w_size != 1) {
    throw("Windchest size is wrong");
  }
  unlink(dir_name);
}
