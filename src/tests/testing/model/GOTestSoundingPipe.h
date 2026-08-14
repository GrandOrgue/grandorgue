/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDINGPIPE_H
#define GOTESTSOUNDINGPIPE_H

#include "GOTest.h"

class GOTestSoundingPipe : public GOCommonControllerTest {

private:
  std::string name = "GOTestSoundingPipe";

  /* GetEffectiveAudioGroupId() reflects live config right away, with no
   * need for PreparePlayback()/UpdateAudioGroup() to have run first. */
  void TestAudioGroup();

public:
  GOTestSoundingPipe() { name = "GOTestSoundingPipe"; }
  virtual ~GOTestSoundingPipe();
  virtual void run();
  std::string GetName();
};

#endif
