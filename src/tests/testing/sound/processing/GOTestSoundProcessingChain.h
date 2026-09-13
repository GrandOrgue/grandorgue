/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDPROCESSINGCHAIN_H
#define GOTESTSOUNDPROCESSINGCHAIN_H

#include "../buffer/GOTestSoundBufferBase.h"

class GOTestSoundProcessingChain : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestEmptyChainLeavesBufferUntouched();
  void TestProcessorsRunInOrder();
  void TestEnsureSetupReachesEveryProcessor();
  void TestResetReachesEveryState();
  void TestEnsureParametersUpToDateReachesEveryMapper();
  void TestInsertProcessorInsertsAtPosition();
  void TestRemoveProcessorReturnsOwnershipAndShiftsOrder();
  void TestRemoveMapperAllowsRemovingItsProcessor();
  void TestClearChainRemovesProcessorsAndMappers();
  void TestIsValidReflectsChainState();
  void TestEnsureSetupNoOpDoesNotInvalidateState();
  void TestChainReusableAfterAllStatesDestroyed();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDPROCESSINGCHAIN_H */
