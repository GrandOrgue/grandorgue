/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsOrgans.h"

#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "GOOrgan.h"
#include "archive/GOArchiveFile.h"
#include "config/GOConfig.h"
#include "midi/MIDIEventDialog.h"

BEGIN_EVENT_TABLE(GOSettingsOrgans, wxPanel)
EVT_LIST_ITEM_SELECTED(ID_ORGANS, GOSettingsOrgans::OnOrganSelected)
EVT_BUTTON(ID_ORGAN_UP, GOSettingsOrgans::OnOrganUp)
EVT_BUTTON(ID_ORGAN_DOWN, GOSettingsOrgans::OnOrganDown)
EVT_BUTTON(ID_ORGAN_TOP, GOSettingsOrgans::OnOrganTop)
EVT_BUTTON(ID_ORGAN_DEL, GOSettingsOrgans::OnOrganDel)
EVT_BUTTON(ID_ORGAN_PROPERTIES, GOSettingsOrgans::OnOrganProperties)
EVT_LIST_ITEM_SELECTED(ID_PACKAGES, GOSettingsOrgans::OnPackageSelected)
EVT_BUTTON(ID_PACKAGE_DEL, GOSettingsOrgans::OnPackageDel)
END_EVENT_TABLE()

GOSettingsOrgans::GOSettingsOrgans(
  GOConfig &settings, GOMidi &midi, wxWindow *parent)
  : wxPanel(parent, wxID_ANY), m_config(settings), m_midi(midi) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *const boxOrgans
    = new wxStaticBoxSizer(wxVERTICAL, this, _("Registered Organs"));

  m_Organs = new wxListView(
    this,
    ID_ORGANS,
    wxDefaultPosition,
    wxSize(100, 200),
    wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
  m_Organs->InsertColumn(0, _("Church"));
  m_Organs->InsertColumn(1, _("Builder"));
  m_Organs->InsertColumn(2, _("Recording"));
  m_Organs->InsertColumn(3, _("MIDI"));
  m_Organs->InsertColumn(4, _("Organ package"));
  m_Organs->InsertColumn(5, _("ODF Path"));
  boxOrgans->Add(m_Organs, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_OrganProperties
    = new wxButton(this, ID_ORGAN_PROPERTIES, _("P&roperties..."));
  m_OrganDown = new wxButton(this, ID_ORGAN_DOWN, _("&Down"));
  m_OrganUp = new wxButton(this, ID_ORGAN_UP, _("&Up"));
  m_OrganTop = new wxButton(this, ID_ORGAN_TOP, _("&Top"));
  m_OrganDel = new wxButton(this, ID_ORGAN_DEL, _("&Delete"));
  buttonSizer->Add(m_OrganDown, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganUp, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganTop, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganDel, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganProperties, 0, wxALIGN_LEFT | wxALL, 5);
  boxOrgans->Add(buttonSizer, 0, wxALL, 5);

  topSizer->Add(boxOrgans, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *const boxPackages
    = new wxStaticBoxSizer(wxVERTICAL, this, _("Packages"));

  m_Packages = new wxListView(
    this,
    ID_PACKAGES,
    wxDefaultPosition,
    wxSize(100, 200),
    wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
  m_Packages->InsertColumn(0, _("Name"));
  m_Packages->InsertColumn(1, _("Path"));
  m_Packages->InsertColumn(2, _("ID"));
  m_Packages->InsertColumn(3, _("Info"));
  boxPackages->Add(m_Packages, 1, wxEXPAND | wxALL, 5);

  buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_PackageDel = new wxButton(this, ID_PACKAGE_DEL, _("&Delete"));
  buttonSizer->Add(m_PackageDel, 0, wxALIGN_LEFT | wxALL, 5);
  boxPackages->Add(buttonSizer, 0, wxALL, 5);

  topSizer->Add(boxPackages, 1, wxEXPAND | wxALL, 5);

  this->SetSizer(topSizer);
  topSizer->Fit(this);

  for (unsigned i = 0; i < m_config.GetOrganList().size(); i++) {
    GOOrgan *o = m_config.GetOrganList()[i];
    wxString title = o->GetChurchName();
    if (!o->IsUsable(m_config))
      title = _("MISSING - ") + title;
    m_Organs->InsertItem(i, title);
    m_Organs->SetItemPtrData(i, (wxUIntPtr)o);
    m_Organs->SetItem(i, 1, o->GetOrganBuilder());
    m_Organs->SetItem(i, 2, o->GetRecordingDetail());
    m_Organs->SetItem(
      i, 3, o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No"));
    m_Organs->SetItem(i, 5, o->GetODFPath());
    if (o->GetArchiveID() != wxEmptyString) {
      const GOArchiveFile *a = m_config.GetArchiveByID(o->GetArchiveID());
      m_Organs->SetItem(i, 4, a ? a->GetName() : o->GetArchiveID());
    }
  }
  if (m_Organs->GetItemCount()) {
    m_Organs->SetColumnWidth(0, 150);
    m_Organs->SetColumnWidth(1, 150);
    m_Organs->SetColumnWidth(2, 250);
    m_Organs->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
    m_Organs->SetColumnWidth(4, wxLIST_AUTOSIZE);
    m_Organs->SetColumnWidth(5, wxLIST_AUTOSIZE);
  }
  m_OrganUp->Disable();
  m_OrganDown->Disable();
  m_OrganTop->Disable();
  m_OrganDel->Disable();
  m_OrganProperties->Disable();

  for (unsigned i = 0; i < m_config.GetArchiveList().size(); i++) {
    GOArchiveFile *a = m_config.GetArchiveList()[i];
    wxString title = a->GetName();
    wxString info = wxEmptyString;
    if (!a->IsUsable(m_config))
      title = _("MISSING - ") + title;
    else if (!a->IsComplete(m_config)) {
      title = _("INCOMPLETE - ") + title;
      for (unsigned i = 0; i < a->GetDependencies().size(); i++)
        if (!m_config.GetArchiveByID(a->GetDependencies()[i], true))
          info += wxString::Format(
            _("requires '%s' "), a->GetDependencyTitles()[i]);
    }
    m_Packages->InsertItem(i, title);
    m_Packages->SetItemPtrData(i, (wxUIntPtr)a);
    m_Packages->SetItem(i, 1, a->GetPath());
    m_Packages->SetItem(i, 2, a->GetID());
    m_Packages->SetItem(i, 3, info);
  }

  if (m_Packages->GetItemCount()) {
    m_Packages->SetColumnWidth(0, wxLIST_AUTOSIZE);
    m_Packages->SetColumnWidth(1, wxLIST_AUTOSIZE);
    m_Packages->SetColumnWidth(2, wxLIST_AUTOSIZE);
    m_Packages->SetColumnWidth(3, wxLIST_AUTOSIZE);
  }
  m_PackageDel->Disable();
}

void GOSettingsOrgans::OnOrganSelected(wxListEvent &event) {
  long index = m_Organs->GetFirstSelected();
  m_OrganDel->Enable();
  m_OrganProperties->Enable();
  if (index <= 0) {
    m_OrganTop->Disable();
    m_OrganUp->Disable();
  } else {
    m_OrganTop->Enable();
    m_OrganUp->Enable();
  }
  if (
    m_Organs->GetItemCount() > 1 && index >= 0
    && index + 1 < m_Organs->GetItemCount())
    m_OrganDown->Enable();
  else
    m_OrganDown->Disable();
}

void GOSettingsOrgans::MoveOrgan(long from, long to) {
  wxListItem item;
  item.SetId(from);
  item.SetMask(-1);
  m_Organs->GetItem(item);
  item.SetId(to);
  m_Organs->InsertItem(item);
  if (to < from)
    from++;
  for (unsigned i = 0; i < 6; i++) {
    item.SetId(from);
    item.SetColumn(i);
    m_Organs->GetItem(item);
    item.SetId(to);
    m_Organs->SetItem(item);
  }
  m_Organs->SetItemPtrData(to, m_Organs->GetItemData(from));
  m_Organs->DeleteItem(from);
  m_Organs->Select(to);
}

void GOSettingsOrgans::OnOrganUp(wxCommandEvent &event) {
  MoveOrgan(m_Organs->GetFirstSelected(), m_Organs->GetFirstSelected() - 1);
}

void GOSettingsOrgans::OnOrganDown(wxCommandEvent &event) {
  MoveOrgan(m_Organs->GetFirstSelected(), m_Organs->GetFirstSelected() + 1);
}

void GOSettingsOrgans::OnOrganTop(wxCommandEvent &event) {
  MoveOrgan(m_Organs->GetFirstSelected(), 0);
}

void GOSettingsOrgans::OnOrganDel(wxCommandEvent &event) {
  if (
    wxMessageBox(
      wxString::Format(
        _("Do you want to remove %s?"),
        m_Organs->GetItemText(m_Organs->GetFirstSelected()).c_str()),
      _("Organs"),
      wxYES_NO | wxICON_EXCLAMATION,
      this)
    == wxYES) {
    m_Organs->DeleteItem(m_Organs->GetFirstSelected());
    m_OrganUp->Disable();
    m_OrganDown->Disable();
    m_OrganTop->Disable();
    m_OrganDel->Disable();
    m_OrganProperties->Disable();
  }
}

void GOSettingsOrgans::OnOrganProperties(wxCommandEvent &event) {
  GOOrgan *o = (GOOrgan *)m_Organs->GetItemData(m_Organs->GetFirstSelected());
  MIDIEventDialog dlg(
    NULL,
    this,
    wxString::Format(
      _("MIDI settings for organ %s"), o->GetChurchName().c_str()),
    m_config,
    &o->GetMIDIReceiver(),
    NULL,
    NULL);
  dlg.RegisterMIDIListener(&m_midi);
  dlg.ShowModal();
  m_Organs->SetItem(
    m_Organs->GetFirstSelected(),
    3,
    o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No"));
}

void GOSettingsOrgans::OnPackageSelected(wxListEvent &event) {
  m_PackageDel->Enable();
}

void GOSettingsOrgans::OnPackageDel(wxCommandEvent &event) {
  const GOArchiveFile *a = (const GOArchiveFile *)m_Packages->GetItemData(
    m_Packages->GetFirstSelected());

  for (long i = 0; i < m_Packages->GetItemCount(); i++) {
    const GOArchiveFile *a1 = (const GOArchiveFile *)m_Packages->GetItemData(i);
    if (a == a1)
      continue;
    if (!a1->IsUsable(m_config))
      continue;
    if (a->GetID() == a1->GetID()) {
      m_Packages->DeleteItem(m_Packages->GetFirstSelected());
      m_PackageDel->Disable();
      return;
    }
  }

  for (unsigned l = m_Organs->GetItemCount(), i = 0; i < l; i++) {
    const GOOrgan *pOrgan = (const GOOrgan *)m_Organs->GetItemData(i);

    if (pOrgan->GetArchiveID() == a->GetID()) {
      wxMessageBox(
        wxString::Format(
          _("'%s' is still used by the organ '%s'"),
          a->GetName().c_str(),
          pOrgan->GetChurchName()),
        _("Delete package"),
        wxOK | wxICON_ERROR,
        this);
      return;
    }
  }
  for (long i = 0; i < m_Packages->GetItemCount(); i++) {
    const GOArchiveFile *a1 = (const GOArchiveFile *)m_Packages->GetItemData(i);
    if (a == a1)
      continue;
    for (unsigned j = 0; j < a1->GetDependencies().size(); j++) {
      if (a->GetID() == a1->GetDependencies()[j]) {
        wxMessageBox(
          wxString::Format(
            _("'%s' is still used by the package '%s'"),
            a->GetName().c_str(),
            a1->GetName()),
          _("Delete package"),
          wxOK | wxICON_ERROR,
          this);
        return;
      }
    }
  }
  m_Packages->DeleteItem(m_Packages->GetFirstSelected());
  m_PackageDel->Disable();
}

void GOSettingsOrgans::Save() {
  ptr_vector<GOOrgan> &listOrgans = m_config.GetOrganList();
  for (unsigned i = 0; i < listOrgans.size(); i++) {
    bool found = false;
    for (long j = 0; j < m_Organs->GetItemCount(); j++)
      if (m_Organs->GetItemData(j) == (wxUIntPtr)listOrgans[i])
        found = true;
    if (!found)
      delete listOrgans[i];
    listOrgans[i] = 0;
  }
  listOrgans.clear();
  for (long i = 0; i < m_Organs->GetItemCount(); i++)
    listOrgans.push_back((GOOrgan *)m_Organs->GetItemData(i));

  ptr_vector<GOArchiveFile> &listPackages = m_config.GetArchiveList();
  for (unsigned i = 0; i < listPackages.size(); i++) {
    bool found = false;
    for (long j = 0; j < m_Packages->GetItemCount(); j++)
      if (m_Packages->GetItemData(j) == (wxUIntPtr)listPackages[i])
        found = true;
    if (!found)
      delete listPackages[i];
    listPackages[i] = 0;
  }
  listPackages.clear();
  for (long i = 0; i < m_Packages->GetItemCount(); i++)
    listPackages.push_back((GOArchiveFile *)m_Packages->GetItemData(i));
}
