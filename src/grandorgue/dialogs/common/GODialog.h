/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIALOG_H
#define GODIALOG_H

#include <wx/sizer.h>

#include "gui/primitives/go_gui_utils.h"
#include "help/GOHelpRequestor.h"

#include "GODialogCloser.h"

class wxSizer;

template <class DialogClass>
class GODialog : public DialogClass, public GODialogCloser {
private:
  static const wxString WX_EMPTY;

  const wxString m_name;
  wxSizer *p_ButtonSizer;

  void OnHelp(wxCommandEvent &event) {
    const wxString &helpSuffix = GetHelpSuffix();
    const wxString helpSection
      = m_name + (helpSuffix.IsEmpty() ? WX_EMPTY : wxT(".") + helpSuffix);

    GOHelpRequestor::DisplayHelp(helpSection, wxDialog::IsModal());
  }

protected:
  GODialog(
    wxWindow *win,
    const wxString &name,  // not translated
    const wxString &title, // translated
    long addStyle = 0,
    long buttonFlags = wxOK | wxCANCEL | wxHELP)
    : DialogClass(
      win,
      wxID_ANY,
      title,
      wxDefaultPosition,
      wxDefaultSize,
      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | addStyle),
      GODialogCloser(this),
      m_name(name) {
    wxDialog::SetIcon(get_go_icon());
    p_ButtonSizer = wxDialog::CreateButtonSizer(buttonFlags);
    this->template SetLayoutAdaptationMode(wxDIALOG_ADAPTATION_MODE_ENABLED);
  }

  wxSizer *GetButtonSizer() const { return p_ButtonSizer; }

  /**
   * For complex dialogs return the current help subsection.
   * The default implementation is for simple dialogs without subsections
   * @return the current subsection name or an emptyy string
   */
  virtual const wxString &GetHelpSuffix() const { return WX_EMPTY; }

  DECLARE_EVENT_TABLE()
};

template <class DialogClass>
const wxString GODialog<DialogClass>::WX_EMPTY = wxEmptyString;

BEGIN_EVENT_TABLE_TEMPLATE1(GODialog, DialogClass, DialogClass)
EVT_BUTTON(wxID_HELP, GODialog::OnHelp)
END_EVENT_TABLE()

#endif /* GODIALOG_H */
