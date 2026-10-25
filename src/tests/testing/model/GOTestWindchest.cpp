/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
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

#include "GOTestScope.h"

GOTestWindchest::~GOTestWindchest() {}

void GOTestWindchest::run() { GO_RUN_TEST(TestWindchestBasics()) }

void GOTestWindchest::TestWindchestBasics() {
  std::string message;
  // Check the size of Windchest is correct
  unsigned w_size;
  w_size = this->controller->AddWindchest(
    std::make_unique<GOWindchest>(*this->controller));
  message = "Windchest size is wrong";
  this->GOAssert(w_size == 1, message);

  w_size = this->controller->AddWindchest(
    std::make_unique<GOWindchest>(*this->controller));
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

  // Check enclosure amplitude values from midi ones
  std::unique_ptr<GOEnclosure> oEnclosure
    = std::make_unique<GOEnclosure>(*this->controller);
  GOEnclosure *enclosure = oEnclosure.get();
  // windchest->AddEnclosure() only registers enclosure in a non-owning
  // std::vector (GOWindchest::m_enclosure) for volume grouping; ownership
  // (and destruction) comes from the model's own ptr_vector, registered
  // via controller->AddEnclosure().
  controller->AddEnclosure(std::move(oEnclosure));
  windchest->AddEnclosure(enclosure);

  // RemoveEnclosure() undoes the last AddEnclosure()
  windchest->RemoveEnclosure(enclosure);
  this->GOAssert(
    windchest->GetEnclosures().empty(),
    "RemoveEnclosure() should remove the enclosure from the windchest");
  windchest->AddEnclosure(enclosure);

  enclosure->SetEnclosureValue(127);
  float amplitude = windchest->GetAmplitude();
  message = "The Windchest amplitude is not 1 but ";
  message = message + std::to_string(amplitude);
  this->GOAssert(amplitude == 1, message);

  enclosure->SetEnclosureValue(0);
  amplitude = windchest->GetAmplitude();
  message = "The Windchest amplitude is not 0 but ";
  message = message + std::to_string(amplitude);
  this->GOAssert(amplitude == 0, message);

  // Check a MIDI value of 50 (50/127)
  enclosure->SetEnclosureValue(50);
  amplitude = windchest->GetAmplitude();
  message = "The Windchest amplitude is not 0.393701 but ";
  message = message + std::to_string(amplitude);
  this->GOAssert(amplitude == 0.393700778f, message);

  // Check Tremulants
  int tremulant_count = windchest->GetTremulantCount();
  message = "The number of Windchest tremulants should be 0!";
  this->GOAssert(tremulant_count == 0, message);

  // Test Ranks
  std::unique_ptr<GORank> oRank = std::make_unique<GORank>(*this->controller);
  GORank *pRank = oRank.get();

  message = "The Rank count for Windchest should be 0";
  this->GOAssert(windchest->GetRankCount() == 0, message);
  // windchest->AddRank() only registers rank in a non-owning std::vector
  // (GOWindchest::m_ranks) for grouping; ownership (and destruction, which
  // transitively frees rank's own owned pipes via GORank::m_Pipes) comes
  // from the model's own ptr_vector, registered via controller->AddRank().
  controller->AddRank(std::move(oRank));
  windchest->AddRank(pRank);
  this->GOAssert(windchest->GetRankCount() == 1, message);

  // Get Rank by Index 0
  GORank *rank_get = windchest->GetRank(0);
  message = "The Rank from GetRank should be the same as the one added!";
  this->GOAssert(rank_get == pRank, message);

  // Get Rank by Index 1
  rank_get = windchest->GetRank(1);
  message = "No Rank should be found with GetRank(1)!";
  this->GOAssert(rank_get == nullptr, message);

  // Test Pipes
  message = "The Pipe count for Windchest should be 0";
  this->GOAssert(pRank->GetPipeCount() == 0, message);
  GOSoundingPipe *pipe = new GOSoundingPipe(
    this->controller, pRank, w_size, 36, 8, 100, 100, false);
  pRank->AddPipe(pipe);

  message = "The Pipe count for Windchest should be 1";
  this->GOAssert(pRank->GetPipeCount() == 1, message);
}

std::string GOTestWindchest::GetName() { return name; }
