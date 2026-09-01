/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDPROCESSORTYPED_H
#define GOTESTSOUNDPROCESSORTYPED_H

#include "../buffer/GOTestSoundBufferBase.h"

class GOTestSoundProcessorTyped : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestOneProcessorTwoIndependentStates();
  void TestTypedDispatch();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDPROCESSORTYPED_H */
