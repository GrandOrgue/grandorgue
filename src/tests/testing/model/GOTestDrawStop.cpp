#include "GOTestDrawStop.h"

#include "model/GOSwitch.h"

std::string GOTestDrawStop::CLASS_NAME = "GOTestDrawStop";
std::string GOTestDrawStop::TestFunctions::TEST_NAME
  = CLASS_NAME + "::Functions";

void GOTestDrawStop::TestFunctions::run() {
  // test grouping functionality
  GOSwitch sw1(*controller);
  GOSwitch sw2(*controller);
  GOSwitch sw3(*controller);
  GOSwitch sw12(*controller);
  GOSwitch sw13(*controller);
  GOSwitch sw23(*controller);

  sw12.SetFunctionType(GODrawstop::FUNCTION_AND);
  sw13.SetFunctionType(GODrawstop::FUNCTION_OR);
  sw23.SetFunctionType(GODrawstop::FUNCTION_XOR);
  sw12.AddControllingDrawstop(&sw1, 1, "sw1");
  sw12.AddControllingDrawstop(&sw2, 2, "sw1");
  sw13.AddControllingDrawstop(&sw1, 1, "sw2");
  sw13.AddControllingDrawstop(&sw3, 2, "sw2");
  sw23.AddControllingDrawstop(&sw2, 1, "sw3");
  sw23.AddControllingDrawstop(&sw3, 2, "sw3");

  GOAssert(!sw12.IsActive(), "false AND false");
  GOAssert(!sw13.IsActive(), "false OR false");
  GOAssert(!sw23.IsActive(), "false XOR false");

  sw1.SetButtonState(true);
  GOAssert(!sw12.IsActive(), "true AND false");
  GOAssert(sw13.IsActive(), "true OR false");
  GOAssert(!sw23.IsActive(), "false XOR false");

  sw1.SetButtonState(false);
  sw2.SetButtonState(true);
  GOAssert(!sw12.IsActive(), "false AND true");
  GOAssert(!sw13.IsActive(), "false OR false");
  GOAssert(sw23.IsActive(), "true XOR false");

  sw1.SetButtonState(true);
  GOAssert(sw12.IsActive(), "true AND true");
  GOAssert(sw13.IsActive(), "true OR false");
  GOAssert(sw23.IsActive(), "true XOR false");

  sw1.SetButtonState(false);
  sw2.SetButtonState(false);
  sw3.SetButtonState(true);
  GOAssert(!sw12.IsActive(), "false AND false");
  GOAssert(sw13.IsActive(), "false OR true");
  GOAssert(sw23.IsActive(), "false XOR true");

  sw1.SetButtonState(true);
  GOAssert(!sw12.IsActive(), "true AND false");
  GOAssert(sw13.IsActive(), "true OR true");
  GOAssert(sw23.IsActive(), "false XOR true");

  sw1.SetButtonState(false);
  sw2.SetButtonState(true);
  GOAssert(!sw12.IsActive(), "false AND true");
  GOAssert(sw13.IsActive(), "false OR true");
  GOAssert(!sw23.IsActive(), "true XOR true");

  sw1.SetButtonState(true);
  GOAssert(sw12.IsActive(), "true AND true");
  GOAssert(sw13.IsActive(), "true OR true");
  GOAssert(!sw23.IsActive(), "true XOR true");

  /*
  std::string message;

  // Check global Switch
  GOSwitch *go_switch(*this->controller);
  message = "The associated manual should be -1 as it is a global switch!";
  this->GOAssert(go_switch->GetAssociatedManualN() == -1, message);

  message = "The switch index in manual should be 0!";
  this->GOAssert(go_switch->GetIndexInManual() == 0, message);
  */

  // Test AND function of switches

  sw23.ClearControllingDrawstops();
  sw13.ClearControllingDrawstops();
  sw12.ClearControllingDrawstops();
}
