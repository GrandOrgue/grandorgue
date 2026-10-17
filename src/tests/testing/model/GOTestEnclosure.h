/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTENCLOSURE_H
#define GOTESTENCLOSURE_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOEnclosure's shelf-EQ configuration (issue #717): the four new
 * accessors, their m_Default* counterparts, and the position-scaled
 * effective gain getters that feed GOSoundEnclosureShelfMapper. Constructs
 * a GOEnclosure directly (same file-I/O-free pattern GOTestWindchest.cpp
 * already uses for GOEnclosure) rather than through Init()/Load(), so ODF/
 * .cmb round-tripping - already untested for the pre-existing
 * AmpMinimumLevel field this mirrors - is out of scope here too.
 */
class GOTestEnclosure : public GOTest {
private:
  static const std::string TEST_NAME;

  /** A freshly constructed enclosure has every shelf field - both current
   * and Default* - at 0, matching m_DefaultAmpMinimumLevel's own always-0
   * constructor default. */
  void TestFreshEnclosureHasZeroShelfConfig();

  /** Each Set()/Get() pair round-trips the exact value passed in. */
  void TestShelfAccessorsRoundTrip();

  /** GetAttenuation() keeps its pre-refactor formula (min/100 at fully
   * closed, 1.0 at fully open, linear in between) after being rewritten on
   * top of InterpolateByMidiValue(). */
  void TestGetAttenuationUnchanged();

  /** GetCurrentLowShelfGainDb()/GetCurrentHighShelfGainDb(): 0 (no-op) at
   * fully open (MIDI 127), -attenuation at fully closed (MIDI 0), the
   * linear midpoint at MIDI 64 - and never positive at any position. */
  void TestCurrentShelfGainDbTracksPosition();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTENCLOSURE_H */
