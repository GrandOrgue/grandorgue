/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
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
#include <wx/string.h>

class GOTestResult {

private:
  wxString m_result_message;
  bool failed;

public:
  GOTestResult();
  GOTestResult(wxString message);
  GOTestResult(wxString message, bool failed);

  const wxString GetMessage() {
    wxString result = "";
    // if (this->isFailed())
    //     result += "\033[1;31m";
    result += m_result_message;
    // if (this->isFailed())
    //     result += "\033[0m";
    return result;
    // return m_result_message;
  }
  bool isFailed() { return failed; }
};

#endif