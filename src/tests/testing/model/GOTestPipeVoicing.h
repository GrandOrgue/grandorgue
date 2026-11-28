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

  /* A change is written to all pipes with the note at once. An untouched
   * pipe reads as it is stored, an inheriting tone balance reads as 0 */
  void TestApplyWritesThroughImmediately();

  /* The values that are already set are not applied again, so the sound is
   * not notified */
  void TestUnchangedValuesAreNotApplied();

  /* The value of a note is empty if the voiced pipes of the note differ */
  void TestValueIsEmptyWhenPipesDiffer();

  /* Revert restores the settings at SetVoicedNodes(), applying only the
   * differing ones, and IsAnyVoicedPipeModified follows */
  void TestRevertRestoresSavedSettings();

  /* ApplySettingValueToAllPipes and ApplySettingRampToMidiNotes */
  void TestAllAndRamp();

  /* Values out of the range are clamped */
  void TestValuesAreClamped();

  /* A pipe value is added to the values set on the upper levels, which stay
   * untouched */
  void TestVoicingWorksOnTopOfUpperLevels();

  /* A pipe reachable from several voiced subtrees is voiced once */
  void TestPipeIsVoicedOnce();

public:
  GOTestPipeVoicing() { name = "GOTestPipeVoicing"; }
  virtual ~GOTestPipeVoicing();
  virtual void run();
  std::string GetName();
};

#endif
