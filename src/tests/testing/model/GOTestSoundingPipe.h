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

  /* A temperament gets the pipe's full MIDI note number, so pipes of the same
   * pitch class in different octaves may get different offsets; a pipe that
   * is not retuned gets none. */
  void TestTemperamentByNoteNumber();

  /* Under a temperament of absolute pitch (MTS-ESP) the organ-wide pitch
   * corrections do not apply, the rank ones do; other temperaments keep both.
   */
  void TestAbsolutePitchTemperament();

public:
  GOTestSoundingPipe() { name = "GOTestSoundingPipe"; }
  virtual ~GOTestSoundingPipe();
  virtual void run();
  std::string GetName();
};

#endif
