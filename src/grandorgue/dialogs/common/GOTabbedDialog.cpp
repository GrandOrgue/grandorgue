/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTabbedDialog.h"

#include <algorithm>

#include <wx/bookctrl.h>
#include <wx/panel.h>

#include "GODialogTab.h"

GOTabbedDialog::GOTabbedDialog(
  wxWindow *win,
  const wxString &name,  // not translated
  const wxString &title, // translated
  long addStyle)
  : GODialog(win, name, title, addStyle) {
  p_book = GetBookCtrl();
  p_book->SetExtraStyle(p_book->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
  wxPropertySheetDialog::GetInnerSizer()->Add(
    GetButtonSizer(), 0, wxEXPAND | wxALL, 5);
}

void GOTabbedDialog::AddTab(
  wxPanel *tab, const wxString &tabName, const wxString &tabTitle) {
  m_TabNames.push_back(tabName);
  p_book->AddPage(tab, tabTitle);
}

void GOTabbedDialog::AddTab(GODialogTab *tab) {
  AddTab(tab, tab->GetName(), tab->GetLabel());
}

const wxString &GOTabbedDialog::GetCurrTabName() const {
  return m_TabNames[p_book->GetSelection()];
}

void GOTabbedDialog::NavigateToTab(const wxString &tabName) {
  // find the page number by it's name
  auto begin = m_TabNames.begin();
  auto end = m_TabNames.end();
  auto ptr = std::find(begin, end, tabName);

  if (ptr != end) // found
    p_book->SetSelection(ptr - begin);
}

bool GOTabbedDialog::TransferDataToWindow() {
  return p_book->TransferDataToWindow();
}

bool GOTabbedDialog::Validate() { return p_book->Validate(); }

bool GOTabbedDialog::TransferDataFromWindow() {
  return p_book->TransferDataFromWindow();
}
