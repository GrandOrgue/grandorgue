/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIALOGTAB_H
#define GODIALOGTAB_H

#include <wx/panel.h>

class wxBookCtrlBase;

class GOTabbedDialog;

class GODialogTab : public wxPanel {
private:
  wxBookCtrlBase &m_book;

public:
  GODialogTab(
    GOTabbedDialog *pDlg, const wxString &name, const wxString &label);

  /**
   * Makes this tab selected in the book control and shows the erroe message box
   * @param msgTitle - the title string to show at the title bar of this box
   * @param msgText - the text of the error message to show on the box
   */
  void ShowErrorMessage(const wxString &msgTitle, const wxString &msgText);
};

#endif /* GODIALOGTAB_H */
