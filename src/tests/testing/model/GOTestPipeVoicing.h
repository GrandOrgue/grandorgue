/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPIPEVOICING_H
#define GOTESTPIPEVOICING_H

#include "GOTest.h"

class GOTestPipeVoicing : public GOCommonControllerTest {
private:
  std::string name = "GOTestPipeVoicing";

  /* A change is written to all pipes with the note at once, and an
   * untouched pipe reads as 0 */
  void TestOffsetsWriteThroughImmediately();

  /* The displayed value comes from the first selected pipe of the note */
  void TestFirstPipeIsDisplayed();

  /* Reset restores the values at Select() and IsModified follows */
  void TestResetRestoresSelectedValues();

  /* SetAllOffsets and SetRamp */
  void TestAllAndRamp();

  /* Values out of the range are clamped */
  void TestOffsetsAreClamped();

  /* A pipe offset is added to the values set on the upper levels, which stay
   * untouched */
  void TestOffsetsWorkOnTopOfUpperLevels();

  /* A pipe reachable from two selected nodes is voiced once */
  void TestPipeIsSelectedOnce();

public:
  GOTestPipeVoicing() { name = "GOTestPipeVoicing"; }
  virtual ~GOTestPipeVoicing();
  virtual void run();
  std::string GetName();
};

#endif
