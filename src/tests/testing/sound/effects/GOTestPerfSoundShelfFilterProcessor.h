/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDSHELFFILTERPROCESSOR_H
#define GOTESTPERFSOUNDSHELFFILTERPROCESSOR_H

#include "../buffer/GOTestPerfSoundBufferBase.h"

#include <string>

/**
 * Measures GOSoundShelfFilterProcessor::Process()'s per-frame throughput in
 * the three shapes it takes depending on which bands are configured: both
 * bands not isActive (the bypass early-out, the common case for an enclosure
 * with no shelf config or one that is fully open), only the low band
 * configured (one band's processSample() loop), and both bands configured
 * (low then high, in series - the most expensive shape).
 */
class GOTestPerfSoundShelfFilterProcessor : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  /** Both bands not isActive: Process() leaves the buffer untouched. */
  void TestPerfProcessBothBandsNoop();

  /** Only the low band is configured: one processSample() loop per frame. */
  void TestPerfProcessOnlyLowBandConfigured();

  /** Both bands are configured: low then high, in series. */
  void TestPerfProcessBothBandsConfigured();

public:
  std::string GetName() override { return TEST_NAME; }
  bool setUp() override;
  void run() override;
  bool tearDown() override;
};

#endif /* GOTESTPERFSOUNDSHELFFILTERPROCESSOR_H */
