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
#include "model/GOEnclosure.h"
#include "model/GORank.h"
#include "model/GOWindchest.h"
#include "model/pipe-config/GOPipeConfigTreeNode.h"

#include "GOTestScope.h"

GOTestOrganModel::~GOTestOrganModel() {}

void GOTestOrganModel::TestAssertSoundRoutingForNoOpWithoutSoundSystem() {
  // The fixture's controller is never started, so p_SoundSystem is null -
  // AssertSoundRoutingFor() must not assert for any argument, including a
  // windchest/audio group pair that is not (and could never be) routable.
  controller->AssertSoundRoutingFor(1, 0);
  controller->AssertSoundRoutingFor(999, 999);
}

void GOTestOrganModel::TestCollectWindchestsForNode() {
  std::unique_ptr<GOWindchest> oWindchest1
    = std::make_unique<GOWindchest>(*controller);
  std::unique_ptr<GOWindchest> oWindchest2
    = std::make_unique<GOWindchest>(*controller);
  GOWindchest *pWindchest1 = oWindchest1.get();
  const unsigned windchestN1 = controller->AddWindchest(std::move(oWindchest1));
  const unsigned windchestN2 = controller->AddWindchest(std::move(oWindchest2));
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

void GOTestOrganModel::TestRemoveLastAdded() {
  const unsigned nWindchests = controller->GetWindchestCount();
  const unsigned nRanks = controller->GetRankCount();
  const unsigned nEnclosures = controller->GetEnclosureCount();
  std::set<unsigned> windchestNsBefore;

  controller->CollectWindchestsForNode(
    controller->GetRootPipeConfigNode(), windchestNsBefore);

  const unsigned windchestN1
    = controller->AddWindchest(std::make_unique<GOWindchest>(*controller));
  std::unique_ptr<GOWindchest> oWindchest2
    = std::make_unique<GOWindchest>(*controller);
  GOWindchest *const pWindchest2 = oWindchest2.get();
  std::unique_ptr<GORank> oRank = std::make_unique<GORank>(*controller);
  GORank *const pRank = oRank.get();
  std::unique_ptr<GOEnclosure> oEnclosure
    = std::make_unique<GOEnclosure>(*controller);
  GOEnclosure *const pEnclosure = oEnclosure.get();

  controller->AddWindchest(std::move(oWindchest2));
  controller->AddRank(std::move(oRank));
  controller->AddEnclosure(std::move(oEnclosure));

  controller->RemoveEnclosure(pEnclosure);
  GOAssert(
    controller->GetEnclosureCount() == nEnclosures,
    "RemoveEnclosure() should restore the enclosure count");
  controller->RemoveRank(pRank);
  GOAssert(
    controller->GetRankCount() == nRanks,
    "RemoveRank() should restore the rank count");
  controller->RemoveWindchest(pWindchest2);
  GOAssert(
    controller->GetWindchestCount() == nWindchests + 1,
    "RemoveWindchest() should delete only the last windchest");

  std::set<unsigned> windchestNsAfter;
  std::set<unsigned> windchestNsExpected = windchestNsBefore;

  windchestNsExpected.insert(windchestN1);
  controller->CollectWindchestsForNode(
    controller->GetRootPipeConfigNode(), windchestNsAfter);
  GOAssert(
    windchestNsAfter == windchestNsExpected,
    "CollectWindchestsForNode() should not find the removed windchest");
}

void GOTestOrganModel::run() {
  GO_RUN_TEST(TestAssertSoundRoutingForNoOpWithoutSoundSystem())
  GO_RUN_TEST(TestCollectWindchestsForNode())
  GO_RUN_TEST(TestRemoveLastAdded())
  GO_RUN_TEST(TestOrganModifiedFlag())
}

void GOTestOrganModel::TestOrganModifiedFlag() {
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
