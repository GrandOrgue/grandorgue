/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOG_H
#define GOLOG_H

#include <wx/log.h>

class GOLogWindow;

class GOLog : public wxLog {
private:
  GOLogWindow *m_LogWindow;

  wxString m_CurrentFileName;

protected:
  void DoLogTextAtLevel(wxLogLevel level, const wxString &msg);

public:
  GOLog(wxWindow *parent);
  virtual ~GOLog();

  const wxString &GetCurrentFileName() const { return m_CurrentFileName; }
  void SetCurrentFileName(const wxString &fileName) {
    m_CurrentFileName = fileName;
  }
  void ClearCurrentFileName() { m_CurrentFileName = wxEmptyString; }
};

#endif
