#ifndef GOTESTDRAWSTOP_H
#define GOTESTDRAWSTOP_H

#include "GOTest.h"

class GOTestDrawStop {
private:
  static std::string CLASS_NAME;

  class TestFunctions : public GOCommonControllerTest {
  private:
    static std::string TEST_NAME;

  public:
    virtual void run() override;
    std::string GetName() override { return TEST_NAME; }
  };

  class TestInternalState : public GOCommonControllerTest {
  private:
    static std::string TEST_NAME;

  public:
    virtual void run() override;
    std::string GetName() override { return TEST_NAME; }
  };

  TestFunctions testFunctions;
  TestInternalState testInternalState;
};

#endif /* GOTESTDRAWSTOP_H */
