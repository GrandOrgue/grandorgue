/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTDIVISIONALSETTER_H
#define GOTESTDIVISIONALSETTER_H

#include "GOTest.h"

// Regression tests for
// https://github.com/GrandOrgue/grandorgue/discussions/2552: the Setter's
// banked Divisionals panel did not respond to Set on organs without a Pedal.
class GOTestDivisionalSetter {
private:
  static std::string CLASS_NAME;

  class TestWithoutPedal : public GOCommonControllerTest {
  private:
    static std::string TEST_NAME;

  public:
    virtual void run() override;
    std::string GetName() override { return TEST_NAME; }
  };

  class TestWithPedal : public GOCommonControllerTest {
  private:
    static std::string TEST_NAME;

  public:
    virtual void run() override;
    std::string GetName() override { return TEST_NAME; }
  };

  class TestDivisionalCoupler : public GOCommonControllerTest {
  private:
    static std::string TEST_NAME;

  public:
    virtual void run() override;
    std::string GetName() override { return TEST_NAME; }
  };

  TestWithoutPedal testWithoutPedal;
  TestWithPedal testWithPedal;
  TestDivisionalCoupler testDivisionalCoupler;
};

#endif /* GOTESTDIVISIONALSETTER_H */
