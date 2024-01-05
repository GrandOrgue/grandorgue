/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTRESULT_H
#define GOTESTRESULT_H

#include "GOTest.h"
#include <vector>
#include <wx/string.h>

class GOTestResult {

private:
  wxString m_result_message = wxEmptyString;
  bool failed;

public:
  GOTestResult();
  GOTestResult(wxString message);
  GOTestResult(wxString message, bool failed);

  const wxString GetMessage() {
    wxString result = wxEmptyString;
    result += m_result_message;
    return result;
    // return m_result_message;
  }
  bool isFailed();
};

#endif