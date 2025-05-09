/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTABBEDDIALOG_H
#define GOTABBEDDIALOG_H

#include <vector>

#include <wx/propdlg.h>

#include "GODialog.h"

class wxBookCtrlBase;
class wxPanel;

class GODialogTab;

class GOTabbedDialog : public GODialog<wxPropertySheetDialog> {
private:
  std::vector<wxString> m_TabNames;
  wxBookCtrlBase *p_book;

protected:
  GOTabbedDialog(
    wxWindow *win,
    const wxString &name,  // not translated
    const wxString &title, // translated
    GODialogSizeSet &dialogSizes,
    const wxString dialogSelector,
    long addStyle = 0,
    long buttonFlags = DEFAULT_BUTTON_FLAGS);

  void AddTab(wxPanel *tab, const wxString &tabName, const wxString &tabTitle);
  void AddTab(GODialogTab *tab);

  void ApplyAdditionalSizes(const GOAdditionalSizeKeeper &sizeKeeper) override;
  void CaptureAdditionalSizes(
    GOAdditionalSizeKeeper &sizeKeeper) const override;

public:
  const wxString &GetCurrTabName() const;
  wxBookCtrlBase *GetBook() const { return p_book; }

  const wxString &GetHelpSuffix() const override { return GetCurrTabName(); }

  void NavigateToTab(const wxString &tabName);

  virtual bool TransferDataToWindow() override;
  virtual bool Validate() override;
  virtual bool TransferDataFromWindow() override;
};

#endif /* GOTABBEDDIALOG_H */
