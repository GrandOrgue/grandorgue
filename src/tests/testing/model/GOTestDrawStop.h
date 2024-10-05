#ifndef GOTESTDRAWSTOP_H
#define GOTESTDRAWSTOP_H

#include "GOTest.h"

class GOTestDrawStop : public GOCommonControllerTest {

private:
  std::string name = "GOTestDrawStop";

public:
  GOTestDrawStop() {}
  virtual ~GOTestDrawStop() {}
  virtual void run();
  std::string GetName() { return name; }
};

#endif /* GOTESTDRAWSTOP_H */
