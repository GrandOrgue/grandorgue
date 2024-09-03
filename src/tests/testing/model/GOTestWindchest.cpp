/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <iostream>

#include "GOTest.h"
#include "GOTestCollection.h"
#include "GOTestException.h"
#include "GOTestWindchest.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOEnclosure.h"
#include "model/GORank.h"
#include "model/GOSoundingPipe.h"
#include "model/GOWindchest.h"

GOTestWindchest::~GOTestWindchest() {}

void GOTestWindchest::run() {
  std::string message;
  // Check the size of Windchest is correct
  unsigned w_size;
  w_size = this->controller->AddWindchest(new GOWindchest(*this->controller));
  message = "Windchest size is wrong";
  this->GOAssert(w_size == 1, message);

  w_size = this->controller->AddWindchest(new GOWindchest(*this->controller));
  message = "Windchest size is wrong";
  this->GOAssert(w_size == 2, message);

  // Check the Windchest count function
  w_size = this->controller->GetWindchestCount();
  message = "Windchest count is wrong";
  this->GOAssert(w_size == 2, message);

  // Check if name is void
  GOWindchest *windchest = this->controller->GetWindchest(0);
  const wxString name = windchest->GetName();
  message = "Windchest name should be void (no configuration set)";
  this->GOAssert(name == "", message);

  // Check enclosure volume values from midi ones
  GOEnclosure *enclosure = new GOEnclosure(*this->controller);
  windchest->AddEnclosure(enclosure);

  enclosure->Set(127);
  float volume = windchest->GetVolume();
  message = "The Windchest volume is not 1 but ";
  message = message + std::to_string(volume);
  this->GOAssert(volume == 1, message);

  enclosure->Set(0);
  volume = windchest->GetVolume();
  message = "The Windchest volume is not 0 but ";
  message = message + std::to_string(volume);
  this->GOAssert(volume == 0, message);

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

  // Test Ranks
  GORank *rank = new GORank(*this->controller);
  message = "The Rank count for Windchest should be 0";
  this->GOAssert(windchest->GetRankCount() == 0, message);
  windchest->AddRank(rank);
  this->GOAssert(windchest->GetRankCount() == 1, message);

  // Get Rank by Index 0
  GORank *rank_get = windchest->GetRank(0);
  message = "The Rank from GetRank should be the same as the one added!";
  this->GOAssert(rank_get == rank, message);

  // Get Rank by Index 1
  rank_get = windchest->GetRank(1);
  message = "No Rank should be found with GetRank(1)!";
  this->GOAssert(rank_get == nullptr, message);

  // Test Pipes
  message = "The Pipe count for Windchest should be 0";
  this->GOAssert(rank->GetPipeCount() == 0, message);
  GOSoundingPipe *pipe = new GOSoundingPipe(
    this->controller, rank, w_size, 36, 8, 100, 100, false);
  rank->AddPipe(pipe);

  message = "The Pipe count for Windchest should be 1";
  this->GOAssert(rank->GetPipeCount() == 1, message);
}

std::string GOTestWindchest::GetName() { return name; }
