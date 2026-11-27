/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTTEMPERAMENT_H
#define GOTESTTEMPERAMENT_H

#include "GOTest.h"

/**
 * Tests GOTemperament::GetOffset() of the built-in temperaments for full MIDI
 * note numbers, and of the MTS-ESP temperament with a fake master
 */
class GOTestTemperament : public GOTest {

private:
  static const std::string TEST_NAME;

  void TestOriginalHasNoOffset();
  void TestCentOffsetsRepeatEveryOctave();
  void TestCentOffsetsByPitchClass();
  void TestMtsEspIsListed();
  void TestMtsEspOffsetsByNoteNumber();
  void TestMtsEspStatus();
  void TestMtsEspShortenLongWords();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTTEMPERAMENT_H */
