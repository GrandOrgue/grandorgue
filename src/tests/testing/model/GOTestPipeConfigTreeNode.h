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

  /*
   * A child with no audio group of its own inherits its parent's.
   */
  void TestChildInheritsAudioGroup();

  /*
   * The destructor of a child removes it from its parent.
   */
  void TestChildDestructorDetachesFromParent();

  /*
   * The destructor of a parent resets the parent of the children that are
   * still alive.
   */
  void TestParentDestructorDetachesChildren();

public:
  GOTestPipeConfigTreeNode() { name = "GOTestPipeConfigTreeNode"; }
  virtual ~GOTestPipeConfigTreeNode();
  virtual void run();
  std::string GetName();
};

#endif
