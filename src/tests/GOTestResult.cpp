/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#include "GOTestResult.h"
#include <iostream>
#include <iterator>
#include <vector>

GOTestResult::GOTestResult() {

  m_result_message = wxEmptyString;
  this->failed = false;
}

GOTestResult::GOTestResult(wxString message) : m_result_message(message) {
  failed = false;
}

GOTestResult::GOTestResult(wxString message, bool failed)
  : m_result_message(message) {
  failed = failed;
}

bool GOTestResult::isFailed() { return this->failed; }
