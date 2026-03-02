/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDCALLBACKCONNECTOR_H
#define GOTESTSOUNDCALLBACKCONNECTOR_H

#include "GOTestSoundOrganEngineBase.h"

class GOTestSoundCallbackConnector : public GOTestSoundOrganEngineBase {
private:
  static const std::string TEST_NAME;

  void TestSilenceWithoutEngine();
  void TestConnectDisconnectLifecycle();
  void TestAsyncCallbacksXrun();
  void TestConnectDisconnectCyclesAsyncCallbacks();
  void TestDisconnectWaitsAsyncCallbacks();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDCALLBACKCONNECTOR_H */
