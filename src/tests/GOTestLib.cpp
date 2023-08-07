#include "config/GOConfig.h"
#include <iostream>

#include "GOOrganController.h"
#include "model/GOWindchest.h"

int main() {
  char path[] = ".";
  char *dir_name = mkdtemp(path);
  GOConfig settings(wxT("perftest"));
  GOOrganController *organController = new GOOrganController(settings);

  organController->InitOrganDirectory(dir_name);
  organController->AddWindchest(new GOWindchest(*organController));
  std::cout << "Tests started!" << std::endl;
  std::cout << "Tests (0) succeeded!" << std::endl;
  unlink(dir_name);
  return 0; // You can put a 1 here to see later that it would generate an error
}