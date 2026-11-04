/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPipeConfigTreeNode.h"

#include <memory>

#include "model/pipe-config/GOPipeConfigTreeNode.h"

#include "GOTestScope.h"

GOTestPipeConfigTreeNode::~GOTestPipeConfigTreeNode() {}

void GOTestPipeConfigTreeNode::run() {
  GO_RUN_TEST(TestChildInheritsAudioGroup())
  GO_RUN_TEST(TestChildDestructorDetachesFromParent())
  GO_RUN_TEST(TestParentDestructorDetachesChildren())
}

void GOTestPipeConfigTreeNode::TestChildDestructorDetachesFromParent() {
  GOPipeConfigTreeNode parent(nullptr, this->controller, nullptr);

  GOAssert(parent.GetChildCount() == 0, "A new parent should have no children");
  {
    GOPipeConfigTreeNode child(&parent, this->controller, nullptr);

    GOAssert(parent.GetChildCount() == 1, "The parent should have the child");
  }
  GOAssert(
    parent.GetChildCount() == 0,
    "The destructor of the child should remove it from the parent");
}

void GOTestPipeConfigTreeNode::TestParentDestructorDetachesChildren() {
  GOPipeConfigTreeNode child1(nullptr, this->controller, nullptr);
  GOPipeConfigTreeNode child2(nullptr, this->controller, nullptr);
  auto pParent = std::make_unique<GOPipeConfigTreeNode>(
    nullptr, this->controller, nullptr);

  child1.SetParent(pParent.get());
  child2.SetParent(pParent.get());
  GOAssert(pParent->GetChildCount() == 2, "The parent should have 2 children");
  pParent.reset();
  GOAssert(
    !child1.GetParent() && !child2.GetParent(),
    "The destructor of the parent should reset the parent of its children");
}

void GOTestPipeConfigTreeNode::TestChildInheritsAudioGroup() {
  // GOPipeConfigNode itself requires a non-null GOPipeUpdateCallback for any
  // Set*() call (see GOPipeConfig::SetLargeMember()); GOPipeConfigTreeNode
  // supplies itself as that callback, so it is the node type usable
  // standalone in a test.
  GOPipeConfigTreeNode parent(nullptr, this->controller, nullptr);

  parent.GetPipeConfig().SetAudioGroup("Group A");

  GOPipeConfigTreeNode child(&parent, this->controller, nullptr);

  this->GOAssert(
    child.GetEffectiveAudioGroup() == "Group A",
    "A child with no audio group of its own should inherit its parent's");
}

std::string GOTestPipeConfigTreeNode::GetName() { return name; }
