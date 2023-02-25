/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPortFactory.h"

const std::vector<wxString> GOPortFactory::c_NoApis;

const wxString GOPortFactory::c_NameDelim = wxT(": ");
static const size_t NEME_DELIM_LEN = GOPortFactory::c_NameDelim.length();

wxString GOPortFactory::NameParser::nextComp() {
  wxString res("");

  if (hasMore()) {
    size_t newPos = m_Name.find(c_NameDelim, m_Pos);
    size_t compEnd;

    if (newPos != wxString::npos) {
      compEnd = newPos;
      newPos += NEME_DELIM_LEN;
    } else {
      compEnd = m_Name.length();
    }
    res = m_Name.substr(m_Pos, compEnd - m_Pos);
    m_Pos = newPos;
  }
  return res;
}

void append_name(wxString const &nameComp, wxString &resName) {
  if (!nameComp.IsEmpty()) {
    if (!resName.IsEmpty())
      resName.Append(GOPortFactory::c_NameDelim);
    resName.Append(nameComp);
  }
}

wxString GOPortFactory::ComposeDeviceName(
  wxString const &portName, wxString const &apiName, wxString const &devName) {
  wxString resName;

  if (IsToUsePortName())
    append_name(portName, resName);
  append_name(apiName, resName);
  append_name(devName, resName);
  return resName;
}
