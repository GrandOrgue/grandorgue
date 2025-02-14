/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINGSDIALOGBASE_H
#define GOORGANSETTINGSDIALOGBASE_H

#include "gui/dialogs/common/GOTabbedDialog.h"

class GOOrganSettingsDialogBase : public GOTabbedDialog {
protected:
  GOOrganSettingsDialogBase(
    wxWindow *win,
    const wxString &name,  // not translated
    const wxString &title, // translated
    GODialogSizeSet &dialogSizes,
    const wxString dialogSelector,
    long addStyle = 0,
    long buttonFlags = DEFAULT_BUTTON_FLAGS)
    : GOTabbedDialog(
      win, name, title, dialogSizes, dialogSelector, addStyle, buttonFlags) {}

public:
  virtual void ButtonStatesChanged() = 0;
};

#endif /* GOORGANSETTINGSDIALOGBASE_H */
