/*
 * Copyright 2023-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include "GOTestResult.h"
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

GOTestResult::GOTestResult() {
  m_result_message = "";
  this->failed = false;
}

GOTestResult::GOTestResult(std::string message) : m_result_message(message) {
  failed = false;
}

GOTestResult::GOTestResult(std::string message, bool failed)
  : m_result_message(message) {
  failed = failed;
}
