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

GOTestWindchest::~GOTestWindchest() {}

void GOTestWindchest::run() {
  // Check the size of Windchest is correct
  unsigned w_size;
  w_size = this->controller->AddWindchest(new GOWindchest(*this->controller));
  if (w_size != 1) {
    throw("Windchest size is wrong");
  }
  w_size = this->controller->AddWindchest(new GOWindchest(*this->controller));
  if (w_size != 2) {
    throw("Windchest size is wrong");
  }

  // Check the Windchest count function
  w_size = this->controller->GetWindchestGroupCount();
  if (w_size != 2) {
    throw("Windchest count is wrong");
  }

  // Check if name is void
  GOWindchest *windchest = this->controller->GetWindchest(0);
  const wxString name = windchest->GetName();
  if (name != "") {
    throw("Windchest name should be void (no configuration set)");
  }
}
