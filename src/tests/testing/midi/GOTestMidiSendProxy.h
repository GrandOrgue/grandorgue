/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTMIDISENDPROXY_H
#define GOTESTMIDISENDPROXY_H

#include "GOTest.h"

class GOTestMidiSendProxy : public GOTest {

private:
  static const std::string TEST_NAME;

  void TestDefaultState();
  void TestToSendMidiIsIndependent();
  void TestToRecordMidiIsIndependent();
  void TestStateRestorer();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTMIDISENDPROXY_H */
