/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDBUFFERMUTABLE_H
#define GOTESTPERFSOUNDBUFFERMUTABLE_H

#include "GOTest.h"

#include <string>
#include <vector>

class GOTestPerfSoundBufferMutable : public GOTest {
private:
  static const std::string TEST_NAME;

  std::vector<std::string> m_failedTests;

  void RunAndEvaluateTest(
    const std::string &functionName,
    const struct Baseline &baseline,
    std::function<void()> operation);

  void TestPerfFillWithSilence();
  void TestPerfCopyFrom();
  void TestPerfAddFrom();
  void TestPerfAddFromWithCoefficient();
  void TestPerfCopyChannelFrom();
  void TestPerfAddChannelFrom();
  void TestPerfAddChannelFromWithCoefficient();
  void TestPerfAddChannelFromMonoRecipient();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTPERFSOUNDBUFFERMUTABLE_H */
