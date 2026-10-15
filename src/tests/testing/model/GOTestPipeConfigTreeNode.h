/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPIPECONFIGTREENODE_H
#define GOTESTPIPECONFIGTREENODE_H

#include "GOTest.h"

class GOTestPipeConfigTreeNode : public GOCommonControllerTest {

private:
  std::string name = "GOTestPipeConfigTreeNode";

public:
  GOTestPipeConfigTreeNode() { name = "GOTestPipeConfigTreeNode"; }
  virtual ~GOTestPipeConfigTreeNode();
  virtual void run();
  std::string GetName();
};

#endif
