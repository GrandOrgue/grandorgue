/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIALOG_H
#define GODIALOG_H

#include <wx/propdlg.h>

#include "GODialogCloser.h"

class wxSizer;

class GODialog : public wxPropertySheetDialog, public GODialogCloser {
private:
  static const wxString WX_EMPTY;

  const wxString m_name;
  wxSizer *p_ButtonSizer;

  void OnHelp(wxCommandEvent &event);

protected:
  GODialog(
    wxWindow *win,
    const wxString &name,  // not translated
    const wxString &title, // translated
    long addStyle = 0);

  wxSizer *GetButtonSizer() const { return p_ButtonSizer; }

  /**
   * For complex dialogs return the current help subsection.
   * The default implementation is for simple dialogs without subsections
   * @return the current subsection name or an emptyy string
   */
  virtual const wxString &GetHelpSuffix() const { return WX_EMPTY; }

  DECLARE_EVENT_TABLE()
};

#endif /* GODIALOG_H */
