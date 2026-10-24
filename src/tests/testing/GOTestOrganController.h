/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTORGANCONTROLLER_H
#define GOTESTORGANCONTROLLER_H

#include <string>

#include "GOTest.h"

class GOTestOrganController : public GOCommonControllerTest {

private:
  std::string name = "GOTestOrganController";

public:
  GOTestOrganController() { name = "GOTestOrganController"; }
  virtual ~GOTestOrganController();
  virtual void run();
  std::string GetName();

private:
  /*
   * Loading, saving and clearing of the organ core data.
   */
  void TestOrganCoreData();

  /*
   * The On indicator button does not send its MIDI events in the ordinary
   * lifecycle calls (issue #1762): the controller sends them last by itself.
   * The MIDI order itself is not tested: a recording MIDI send proxy needs a
   * live GOMidiSystem or a configured GOMidiRecorder, which GOTestExe lacks.
   */
  void TestOnStateLifecycleSending();
};

#endif /* GOTESTORGANCONTROLLER_H */
