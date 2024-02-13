/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <cpptrace.hpp>
#include <iostream>

#include "GOTest.h"
#include "GOTestCollection.h"
#include "GOTestException.h"
#include "GOTestWindchest.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOEnclosure.h"
#include "model/GOWindchest.h"

GOTestWindchest::~GOTestWindchest() {}

void GOTestWindchest::run() {
  wxString message;
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

  // Check enclosure volume values from midi ones
  GOEnclosure *enclosure = new GOEnclosure(*this->controller);
  windchest->AddEnclosure(enclosure);

  enclosure->Set(127);
  float volume = windchest->GetVolume();
  if (volume != 1) {
    std::string message = "The Windchest volume is not 1 but ";
    message = message + std::to_string(volume);
    throw GOTestException(message);
  }
  enclosure->Set(0);
  volume = windchest->GetVolume();
  if (volume != 0) {
    std::string message = "The Windchest volume is not 0 but ";
    message = message + std::to_string(volume);
    throw GOTestException(message);
  }

  // Check a MIDI value of 50 (50/127)
  enclosure->Set(50);
  volume = windchest->GetVolume();
  message = "The Windchest volume is not 0.393701 but ";
  message = message + std::to_string(volume);
  this->GOAssert(volume == 0.393700778f, message);

  // Check Tremulants
  int tremulant_count = windchest->GetTremulantCount();
  message = "The number of Windchest tremulants should be 0!";
  this->GOAssert(tremulant_count == 0, message);
}

std::string GOTestWindchest::GetName() { return name; }
