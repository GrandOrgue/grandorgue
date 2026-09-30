/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include <iostream>
#include <set>

#include "GOTest.h"
#include "GOTestCollection.h"
#include "GOTestException.h"
#include "GOTestOrganModel.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"
#include "model/pipe-config/GOPipeConfigTreeNode.h"

GOTestOrganModel::~GOTestOrganModel() {}

void GOTestOrganModel::TestAssertSoundRoutingForNoOpWithoutSoundSystem() {
  // The fixture's controller is never started, so p_SoundSystem is null -
  // AssertSoundRoutingFor() must not assert for any argument, including a
  // windchest/audio group pair that is not (and could never be) routable.
  controller->AssertSoundRoutingFor(1, 0);
  controller->AssertSoundRoutingFor(999, 999);
}

void GOTestOrganModel::TestCollectWindchestsForNode() {
  GOWindchest *pWindchest1 = new GOWindchest(*controller);
  GOWindchest *pWindchest2 = new GOWindchest(*controller);
  const unsigned windchestN1 = controller->AddWindchest(pWindchest1);
  const unsigned windchestN2 = controller->AddWindchest(pWindchest2);
  GOPipeConfigTreeNode nested(
    &pWindchest1->GetPipeConfig(), controller, nullptr);

  std::set<unsigned> fromWindchestNode;

  controller->CollectWindchestsForNode(
    pWindchest1->GetPipeConfig(), fromWindchestNode);
  this->GOAssert(
    fromWindchestNode == std::set<unsigned>{windchestN1},
    "A windchest's own node should resolve to that windchest's number");

  std::set<unsigned> fromNestedNode;

  controller->CollectWindchestsForNode(nested, fromNestedNode);
  this->GOAssert(
    fromNestedNode == std::set<unsigned>{windchestN1},
    "A node nested below a windchest's node should resolve to that "
    "windchest's number");

  std::set<unsigned> fromRoot;

  controller->CollectWindchestsForNode(
    controller->GetRootPipeConfigNode(), fromRoot);
  this->GOAssert(
    fromRoot == (std::set<unsigned>{windchestN1, windchestN2}),
    "The organ root should resolve to every windchest");
}

void GOTestOrganModel::run() {
  TestAssertSoundRoutingForNoOpWithoutSoundSystem();
  TestCollectWindchestsForNode();

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
