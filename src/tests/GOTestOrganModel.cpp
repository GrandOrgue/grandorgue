/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include <iostream>

#include <boost/stacktrace.hpp>
#include <exception>

#include "GOTest.h"
#include "GOTestCollection.h"
#include "GOTestException.h"
#include "GOTestOrganModel.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"

GOTestOrganModel::~GOTestOrganModel() {}

void GOTestOrganModel::run() {
  // Set OrganModel Modified
  this->controller->SetOrganModelModified(true);
  if (!this->controller->IsOrganModified()) {
    std::string message = "Is Modified value is not True";
    throw GOTestException(message);
  }

  this->controller->ResetOrganModified();

  if (this->controller->IsOrganModified()) {
    std::string message = "Is Modified value is not False";
    throw GOTestException(message);
  }

  // Check the NotifyPipeConfigModified function
  this->controller->SetOrganModelModified(false);
  this->controller->NotifyPipeConfigModified();
  if (!this->controller->IsOrganModified()) {
    std::string message
      = "NotifyPipeConfigModified: Is Modified value is not True";
    throw GOTestException(message);
  }
}
