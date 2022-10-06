/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSELECTORGANDIALOG_H_
#define GOSELECTORGANDIALOG_H_

#include <wx/dialog.h>
#include <wx/listctrl.h>

class GOOrgan;
class GOOrganList;

class GOSelectOrganDialog : public wxDialog {
private:
  const GOOrganList &m_OrganList;
  wxListView *m_Organs;

  enum { ID_ORGANS = 200 };

  void OnOK(wxCommandEvent &event);
  void OnDoubleClick(wxListEvent &event);

public:
  GOSelectOrganDialog(
    wxWindow *parent, wxString title, const GOOrganList &organList);

  bool TransferDataToWindow() override;

  const GOOrgan *GetSelection();

  DECLARE_EVENT_TABLE()
};

#endif
