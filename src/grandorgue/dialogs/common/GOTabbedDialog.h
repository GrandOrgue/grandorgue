/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTABBEDDIALOG_H
#define GOTABBEDDIALOG_H

#include <vector>

#include <wx/propdlg.h>

#include "GODialogCloser.h"

class wxBookCtrlBase;
class wxPanel;
class wxSizer;

class GOTabbedDialog : public wxPropertySheetDialog, public GODialogCloser {
private:
  const wxString m_name;
  std::vector<wxString> m_TabNames;
  wxBookCtrlBase *p_book;
  wxSizer *p_ButtonSizer;

  void OnHelp(wxCommandEvent &event);

protected:
  GOTabbedDialog(
    wxWindow *win,
    const wxString &name,  // not translated
    const wxString &title, // translated
    long addStyle = 0);

  wxBookCtrlBase *GetBook() const { return p_book; }
  wxSizer *GetButtonSizer() const { return p_ButtonSizer; }

  void AddTab(wxPanel *tab, const wxString &tabName, const wxString &tabTitle);

public:
  const wxString &GetCurrTabName() const;
  void NavigateToTab(const wxString &tabName);

  DECLARE_EVENT_TABLE()
};

#endif /* GOTABBEDDIALOG_H */
