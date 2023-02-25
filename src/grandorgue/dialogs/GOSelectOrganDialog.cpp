/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSelectOrganDialog.h"

#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "GOOrgan.h"
#include "GOOrganList.h"
#include "archive/GOArchiveFile.h"

BEGIN_EVENT_TABLE(GOSelectOrganDialog, wxDialog)
EVT_BUTTON(wxID_OK, GOSelectOrganDialog::OnOK)
EVT_LIST_ITEM_ACTIVATED(ID_ORGANS, GOSelectOrganDialog::OnDoubleClick)
END_EVENT_TABLE()

GOSelectOrganDialog::GOSelectOrganDialog(
  wxWindow *parent, wxString title, const GOOrganList &organList)
  : wxDialog(
    NULL,
    wxID_ANY,
    title,
    wxDefaultPosition,
    wxSize(600, 480),
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT),
    m_OrganList(organList) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);

  m_Organs = new wxListView(
    this,
    ID_ORGANS,
    wxDefaultPosition,
    wxDefaultSize,
    wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
  m_Organs->InsertColumn(0, _("Church"));
  m_Organs->InsertColumn(1, _("Organ package"));
  m_Organs->InsertColumn(2, _("Organ path"));
  topSizer->Add(m_Organs, 1, wxEXPAND | wxALL, 5);

  m_Organs->SetColumnWidth(0, 150);
  m_Organs->SetColumnWidth(1, 150);
  m_Organs->SetColumnWidth(2, 300);

  topSizer->AddSpacer(5);
  topSizer->Add(
    CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 5);

  SetSizerAndFit(topSizer);
}

bool GOSelectOrganDialog::TransferDataToWindow() {
  for (unsigned i = 0, j = 0; j < m_OrganList.GetOrganList().size(); j++) {
    const GOOrgan *o = m_OrganList.GetOrganList()[j];

    if (o->IsUsable(m_OrganList)) {
      const bool isArchive = o->GetArchiveID() != wxEmptyString;

      m_Organs->InsertItem(i, o->GetChurchName());
      m_Organs->SetItemPtrData(i, (wxUIntPtr)o);
      if (isArchive) {
        const GOArchiveFile *a = m_OrganList.GetArchiveByID(o->GetArchiveID());

        m_Organs->SetItem(i, 1, a ? a->GetName() : o->GetArchiveID());
      }
      m_Organs->SetItem(
        i, 2, isArchive ? o->GetArchivePath() : o->GetODFPath());
      i++;
    }
  }
  return true;
}

void GOSelectOrganDialog::OnOK(wxCommandEvent &event) {
  if (m_Organs->GetItemData(m_Organs->GetFirstSelected()) == 0) {
    wxMessageBox(
      _("Please select an organ"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }
  EndModal(wxID_OK);
}

const GOOrgan *GOSelectOrganDialog::GetSelection() {
  return (const GOOrgan *)m_Organs->GetItemData(m_Organs->GetFirstSelected());
}

void GOSelectOrganDialog::OnDoubleClick(wxListEvent &event) {
  wxCommandEvent e;
  OnOK(event);
}
