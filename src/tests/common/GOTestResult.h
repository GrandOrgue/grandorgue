/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTRESULT_H
#define GOTESTRESULT_H

#include <fstream>
#include <iostream>
#include <string>

#include "GOTest.h"
#include <vector>

class GOTestResult {

private:
  std::string m_result_message;
  bool failed;

public:
  GOTestResult();
  GOTestResult(std::string message);
  GOTestResult(std::string message, bool failed);

  const std::string GetMessage() { return m_result_message; }
  bool isFailed() { return failed; }
};

#endif