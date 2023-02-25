/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLog.h"

#include "GOLogWindow.h"

GOLog::GOLog(wxWindow *parent) : m_LogWindow(NULL) {
  m_LogWindow = new GOLogWindow(parent, wxID_ANY, _("Log messages"));
}

GOLog::~GOLog() {
  if (m_LogWindow) {
    m_LogWindow = NULL;
  }
}

void GOLog::DoLogTextAtLevel(wxLogLevel level, const wxString &msg) {
  if (m_LogWindow) {
    wxString resultMsg = msg;

    if (!m_CurrentFileName.IsEmpty())
      resultMsg
        = resultMsg + wxString::Format(_("; FileName=%s"), m_CurrentFileName);

    m_LogWindow->LogMsg(level, resultMsg, time(0));
  }
}
