/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDORGANENGINESTRESS_H
#define GOTESTSOUNDORGANENGINESTRESS_H

#include "GOTestSoundOrganEngineBase.h"

class GOTestSoundOrganEngineStress : public GOTestSoundOrganEngineBase {
private:
  static const std::string TEST_NAME;

  /*
   * Starts 5 callback threads via m_Connector:
   *   threads 0, 1 → output 0 (xrun)
   *   thread 2     → output 1
   *   threads 3, 4 → output 2 (xrun)
   * Threads run until isRunning is set to false.
   */
  void RunCallbackThreads(
    std::atomic_bool &isRunning, std::vector<std::thread> &outThreads);

  /*
   * Builds engine once (3out/2g/3aux), then runs 100 connect/disconnect cycles
   * with 2ms sleep under xrun load from 5 threads.
   */
  void TestConnectDisconnectCycles();

  /*
   * Runs 100 build/connect/disconnect/stop cycles with 2ms sleep
   * under xrun load from 5 threads.
   */
  void TestBuildStopCycles();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDORGANENGINESTRESS_H */
