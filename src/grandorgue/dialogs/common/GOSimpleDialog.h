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
protected:
  GOSimpleDialog(
    wxWindow *win,
    const wxString &name,  // not translated
    const wxString &title, // translated
    long addStyle = 0,
    long buttonFlags = wxOK | wxCANCEL | wxHELP)
    : GODialog(win, name, title, addStyle, buttonFlags) {}

  void LayoutWithInnerSizer(wxSizer *pInnerSizer);
};

#endif /* GOSIMPLEDIALOG_H */
