/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSwitch.h"
#include "model/GOSwitch.h"

GOTestSwitch::~GOTestSwitch() {}

void GOTestSwitch::run() {
  std::string message;

  // Check global Switch
  GOSwitch goSwitch(*this->controller);
  message = "The associated manual should be -1 as it is a global switch!";
  this->GOAssert(goSwitch.GetAssociatedManualN() == -1, message);

  message = "The switch index in manual should be 0!";
  this->GOAssert(goSwitch.GetIndexInManual() == 0, message);
}

std::string GOTestSwitch::GetName() { return name; }