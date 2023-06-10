/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODialog.h"

#include <wx/app.h>
#include <wx/sizer.h>

#include "help/GOHelpRequestor.h"

#include "GOEvent.h"

const wxString GODialog::WX_EMPTY = wxEmptyString;

BEGIN_EVENT_TABLE(GODialog, wxPropertySheetDialog)
EVT_BUTTON(wxID_HELP, GODialog::OnHelp)
END_EVENT_TABLE()

GODialog::GODialog(
  wxWindow *win,
  const wxString &name,  // not translated
  const wxString &title, // translated
  long addStyle)
  : wxPropertySheetDialog(
    win,
    wxID_ANY,
    title,
    wxDefaultPosition,
    wxDefaultSize,
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | addStyle),
    GODialogCloser(this),
    m_name(name) {
  p_ButtonSizer = CreateButtonSizer(wxOK | wxCANCEL | wxHELP);
  GetInnerSizer()->Add(p_ButtonSizer, 0, wxEXPAND | wxALL, 5);
}

void GODialog::OnHelp(wxCommandEvent &event) {
  const wxString &helpSuffix = GetHelpSuffix();
  const wxString helpSection
    = m_name + (helpSuffix.IsEmpty() ? WX_EMPTY : wxT(".") + helpSuffix);

  GOHelpRequestor::DisplayHelp(helpSection, IsModal());
}
