/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPipeConfigTreeNode.h"

#include "model/pipe-config/GOPipeConfigTreeNode.h"

GOTestPipeConfigTreeNode::~GOTestPipeConfigTreeNode() {}

void GOTestPipeConfigTreeNode::run() {
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
