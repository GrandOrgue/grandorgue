/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODialogTab.h"

#include <wx/bookctrl.h>
#include <wx/msgdlg.h>

#include "GOTabbedDialog.h"

GODialogTab::GODialogTab(
  GOTabbedDialog *pDlg, const wxString &name, const wxString &label)
  : wxPanel(pDlg->GetBook()), m_book(*pDlg->GetBook()) {
  SetName(name);
  SetLabel(label);
}

void GODialogTab::ShowErrorMessage(
  const wxString &msgTitle, const wxString &msgText) {
  int nTab = m_book.FindPage(this);

  if (nTab != wxNOT_FOUND)
    m_book.SetSelection(nTab);
  wxMessageBox(msgText, msgTitle, wxOK | wxCENTRE | wxICON_ERROR);
}
