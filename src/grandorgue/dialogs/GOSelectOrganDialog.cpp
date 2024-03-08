/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSelectOrganDialog.h"

#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "archive/GOArchiveFile.h"
#include "config/GOConfig.h"

#include "GOOrgan.h"
#include "GOOrganList.h"

BEGIN_EVENT_TABLE(GOSelectOrganDialog, GOSimpleDialog)
EVT_LIST_ITEM_ACTIVATED(ID_ORGANS, GOSelectOrganDialog::OnDoubleClick)
END_EVENT_TABLE()

GOSelectOrganDialog::GOSelectOrganDialog(wxWindow *parent, GOConfig &config)
  : GOSimpleDialog(
    nullptr,
    wxT("Load"),
    _("Select organ to load"),
    config.m_DialogSizes,
    wxEmptyString,
    wxDIALOG_NO_PARENT),
    r_OrganList(config) {
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
  LayoutWithInnerSizer(topSizer);
}

bool GOSelectOrganDialog::TransferDataToWindow() {
  for (unsigned i = 0, j = 0; j < r_OrganList.GetOrganList().size(); j++) {
    const GOOrgan *o = r_OrganList.GetOrganList()[j];

    if (o->IsUsable(r_OrganList)) {
      const bool isArchive = o->GetArchiveID() != wxEmptyString;

      m_Organs->InsertItem(i, o->GetChurchName());
      m_Organs->SetItemPtrData(i, (wxUIntPtr)o);
      if (isArchive) {
        const GOArchiveFile *a = r_OrganList.GetArchiveByID(o->GetArchiveID());

        m_Organs->SetItem(i, 1, a ? a->GetName() : o->GetArchiveID());
      }
      m_Organs->SetItem(
        i, 2, isArchive ? o->GetArchivePath() : o->GetODFPath());
      i++;
    }
  }
  return true;
}

bool GOSelectOrganDialog::Validate() {
  bool isValid = true;
  long index = m_Organs->GetFirstSelected();

  if (index < 0 || !m_Organs->GetItemData(index)) {
    wxMessageBox(
      _("Please select an organ"), _("Error"), wxOK | wxICON_ERROR, this);
    isValid = false;
  }
  return isValid;
}

const GOOrgan *GOSelectOrganDialog::GetSelection() {
  return (const GOOrgan *)m_Organs->GetItemData(m_Organs->GetFirstSelected());
}
