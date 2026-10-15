/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTORGANMODEL_H
#define GOTESTORGANMODEL_H

#include "GOTest.h"

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"

class GOTestOrganModel : public GOCommonControllerTest {

private:
  std::string name = "GOTestOrganModel";

  /*
   * AssertSoundRoutingFor() must be a no-op whenever the controller's sound
   * engine has not been started (p_SoundSystem null, e.g. during initial
   * PreparePlayback() at organ-load time) - for any (windchestN, audioGroupId)
   * arguments, including ones that could never be routable.
   */
  void TestAssertSoundRoutingForNoOpWithoutSoundSystem();

  /*
   * CollectWindchestsForNode(): a windchest's own node and a nested node
   * below it both resolve to that windchest's number; the organ root
   * resolves to every windchest.
   */
  void TestCollectWindchestsForNode();

public:
  GOTestOrganModel() { name = "GOTestOrganModel"; }
  virtual ~GOTestOrganModel();
  virtual void run();
  std::string GetName() { return name; };
};

#endif
