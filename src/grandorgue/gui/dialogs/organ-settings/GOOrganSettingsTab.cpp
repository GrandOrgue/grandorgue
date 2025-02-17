/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganSettingsTab.h"

#include <wx/intl.h>

#include "gui/dialogs/common/GOTabbedDialog.h"

#include "GOEvent.h"
#include "GOOrganSettingsDialogBase.h"

GOOrganSettingsTab::GOOrganSettingsTab(
  GOOrganSettingsDialogBase *pDlg, const wxString &name, const wxString &label)
  : GODialogTab(pDlg, name, label), p_dlg(pDlg) {}

void GOOrganSettingsTab::NotifyButtonStatesChanged() {
  p_dlg->ButtonStatesChanged();
}

void GOOrganSettingsTab::NotifyModified(bool newValue) {
  SetModified(newValue);
  NotifyButtonStatesChanged();
}

bool GOOrganSettingsTab::CheckForUnapplied() {
  bool res = IsModified();

  if (res)
    GOMessageBox(
      _("Please apply or discard changes first"),
      _("Error"),
      wxOK | wxICON_ERROR,
      p_dlg);
  return res;
}
