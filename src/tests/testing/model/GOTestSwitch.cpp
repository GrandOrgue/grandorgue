/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSwitch.h"
#include "model/GOSwitch.h"

GOTestSwitch::~GOTestSwitch() {}

void GOTestSwitch::run() {
  std::string message;

  // Check global Switch
  GOSwitch *go_switch = new GOSwitch(*this->controller);
  message = "The associated manual should be -1 as it is a global switch!";
  this->GOAssert(go_switch->GetAssociatedManualN() == -1, message);

  message = "The switch index in manual should be 0!";
  this->GOAssert(go_switch->GetIndexInManual() == 0, message);
}

std::string GOTestSwitch::GetName() { return name; }