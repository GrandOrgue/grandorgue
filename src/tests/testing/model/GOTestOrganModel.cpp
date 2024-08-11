/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include <iostream>

#include "GOTest.h"
#include "GOTestCollection.h"
#include "GOTestException.h"
#include "GOTestOrganModel.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"

GOTestOrganModel::~GOTestOrganModel() {}

void GOTestOrganModel::run() {
  std::string message;
  // Set OrganModel Modified
  this->controller->SetOrganModelModified(true);
  message = "Is Organ Modified value is not True";
  this->GOAssert(this->controller->IsOrganModified(), message);

  this->controller->ResetOrganModified();
  message = "Is Organ Modified value is not False";
  this->GOAssert(!this->controller->IsOrganModified(), message);

  // Check the NotifyPipeConfigModified function
  this->controller->SetOrganModelModified(false);
  this->controller->NotifyPipeConfigModified();
  message = "NotifyPipeConfigModified: Is Modified value is not True";
  this->GOAssert(this->controller->IsOrganModified(), message);
}
