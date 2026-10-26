/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDENCLOSURESHELFMAPPER_H
#define GOTESTPERFSOUNDENCLOSURESHELFMAPPER_H

#include <string>
#include <vector>

#include "GOTest.h"

/**
 * Measures GOSoundEnclosureShelfMapper::EnsureParametersUpToDate()'s
 * per-call throughput in the three shapes it actually takes on the audio
 * thread: nothing changed since the previous round (the steady-state case,
 * true for almost every round), only the high band's config changed (one
 * band recomputes), and both bands' config changed together (both
 * recompute - the same cost EnsureParametersUpToDate() pays on a position
 * change, per GOSoundEnclosureShelfMapper's caching contract, covered
 * functionally by GOTestSoundEnclosureShelfMapper).
 */
class GOTestPerfSoundEnclosureShelfMapper : public GOTest {
private:
  static const std::string TEST_NAME;

  std::vector<std::string> m_failedTests;

  /** Steady state: neither band's config nor the enclosure's position
   * changes between calls, so every call takes the cheap comparison-only
   * path with no computeCoeffs() call at all. */
  void TestPerfNothingChanged();

  /** The high band's frequency changes every call, so every call recomputes
   * the high band and leaves the low band untouched. */
  void TestPerfOnlyHighShelfChanged();

  /** Both bands' frequencies change every call, so every call recomputes
   * both bands. */
  void TestPerfHighAndLowShelfChanged();

public:
  GOTestPerfSoundEnclosureShelfMapper() : GOTest(GOTest::PERF) {}
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  void run() override;
  bool tearDown() override;
};

#endif /* GOTESTPERFSOUNDENCLOSURESHELFMAPPER_H */
