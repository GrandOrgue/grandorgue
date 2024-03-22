/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTabbedDialog.h"

#include <algorithm>

#include <wx/bookctrl.h>
#include <wx/panel.h>

#include "size/GOAdditionalSizeKeeperProxy.h"

#include "GODialogTab.h"

GOTabbedDialog::GOTabbedDialog(
  wxWindow *win,
  const wxString &name,  // not translated
  const wxString &title, // translated
  GODialogSizeSet &dialogSizes,
  const wxString dialogSelector,
  long addStyle)
  : GODialog(win, name, title, dialogSizes, dialogSelector, addStyle) {
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

void GOTabbedDialog::ApplyAdditionalSizes(
  const GOAdditionalSizeKeeper &sizeKeeper) {
  for (unsigned l = p_book->GetPageCount(), i = 0; i < l; i++) {
    GODialogTab *tab = dynamic_cast<GODialogTab *>(p_book->GetPage(i));

    if (tab) {
      GOAdditionalSizeKeeperProxy proxy(
        const_cast<GOAdditionalSizeKeeper &>(sizeKeeper), tab->GetName());

      tab->ApplyAdditionalSizes(proxy);
    }
  }
}

void GOTabbedDialog::CaptureAdditionalSizes(
  GOAdditionalSizeKeeper &sizeKeeper) const {
  for (unsigned l = p_book->GetPageCount(), i = 0; i < l; i++) {
    const GODialogTab *tab
      = dynamic_cast<const GODialogTab *>(p_book->GetPage(i));

    if (tab) {
      GOAdditionalSizeKeeperProxy proxy(sizeKeeper, tab->GetName());

      tab->CaptureAdditionalSizes(proxy);
    }
  }
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
