#include "config/GOConfig.h"
#include <iostream>
#include <wx/app.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/stopwatch.h>

int main() {
  GOConfig *m_config = new GOConfig(wxT("Test"));
  std::cout << m_config;
  std::cout << "Tests started!" << std::endl;
  std::cout << "Tests (0) succeeded!" << std::endl;
  return 0; // You can put a 1 here to see later that it would generate an error
}