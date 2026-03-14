/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGuiLog.h"

#include "frames/GOLogWindow.h"

GOGuiLog::GOGuiLog(wxWindow *parent) : m_LogWindow(NULL) {
  m_LogWindow = new GOLogWindow(parent, wxID_ANY, _("Log messages"));
}

GOGuiLog::~GOGuiLog() {
  if (m_LogWindow) {
    m_LogWindow = NULL;
  }
}

void GOGuiLog::DoLogTextAtLevel(wxLogLevel level, const wxString &msg) {
  if (m_LogWindow) {
    wxString resultMsg = msg;

    if (!m_CurrentFileName.IsEmpty())
      resultMsg
        = resultMsg + wxString::Format(_("; FileName=%s"), m_CurrentFileName);

    m_LogWindow->LogMsg(level, resultMsg, time(0));
  }
}
