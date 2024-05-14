/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSIMPLEDIALOG_H
#define GOSIMPLEDIALOG_H

#include <wx/dialog.h>

#include "GODialog.h"

class wxSizer;

class GOSimpleDialog : public GODialog<wxDialog> {
private:
  wxSizer *p_ContentSizer;

protected:
  GOSimpleDialog(
    wxWindow *parent,
    const wxString &name,  // not translated
    const wxString &title, // translated
    GODialogSizeSet &dialogSizes,
    const wxString dialogSelector = wxEmptyString,
    long addStyle = 0,
    long buttonFlags = DEFAULT_BUTTON_FLAGS);

  void LayoutWithInnerSizer(wxSizer *pInnerSizer);
};

#endif /* GOSIMPLEDIALOG_H */
