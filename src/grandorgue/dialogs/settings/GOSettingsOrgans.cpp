/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsOrgans.h"

#include <algorithm>
#include <set>
#include <unordered_map>

#include <wx/button.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "GOOrgan.h"
#include "archive/GOArchiveFile.h"
#include "config/GOConfig.h"
#include "dialogs/midi-event/GOMidiEventDialog.h"

static wxString EMPTY_STRING = wxEmptyString;

BEGIN_EVENT_TABLE(GOSettingsOrgans, wxPanel)
EVT_LIST_ITEM_FOCUSED(ID_ORGANS, GOSettingsOrgans::OnOrganFocused)
EVT_LIST_ITEM_SELECTED(ID_ORGANS, GOSettingsOrgans::OnOrganSelected)
EVT_LIST_ITEM_DESELECTED(ID_ORGANS, GOSettingsOrgans::OnOrganSelected)
EVT_BUTTON(ID_ORGAN_UP, GOSettingsOrgans::OnOrganUp)
EVT_BUTTON(ID_ORGAN_DOWN, GOSettingsOrgans::OnOrganDown)
EVT_BUTTON(ID_ORGAN_TOP, GOSettingsOrgans::OnOrganTop)
EVT_BUTTON(ID_ORGAN_DEL, GOSettingsOrgans::OnOrganDel)
EVT_BUTTON(ID_ORGAN_PROPERTIES, GOSettingsOrgans::OnOrganProperties)
END_EVENT_TABLE()

GOSettingsOrgans::GOSettingsOrgans(
  GOConfig &settings, GOMidi &midi, wxWindow *parent)
  : wxPanel(parent, wxID_ANY),
    m_config(settings),
    m_midi(midi),
    m_OrigOrganList(m_config.GetOrganList()) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);
  wxGridBagSizer *const gbSizer = new wxGridBagSizer(5, 5);

  m_Organs = new wxListView(
    this,
    ID_ORGANS,
    wxDefaultPosition,
    //    wxSize(400, 200),
    wxDefaultSize,
    wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
  m_Organs->InsertColumn(0, _("Name"));
  m_Organs->InsertColumn(1, _("MIDI"));
  m_Organs->InsertColumn(2, _("Path"));
  m_Organs->SetColumnWidth(0, 200);
  m_Organs->SetColumnWidth(1, 50);
  m_Organs->SetColumnWidth(2, 0);
  m_Organs->SetColumnWidth(2, 300);

  gbSizer->Add(
    m_Organs, wxGBPosition(0, 0), wxGBSpan(1, 4), wxALL | wxEXPAND, 5);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Builder:")),
    wxGBPosition(1, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_Builder = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_Builder, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Recording:")),
    wxGBPosition(2, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_Recording = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_Recording, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Organ Hash:")),
    wxGBPosition(3, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_OrganHash = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_OrganHash, wxGBPosition(3, 1), wxDefaultSpan, wxEXPAND);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Package Id:")),
    wxGBPosition(4, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_PackageId = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_PackageId, wxGBPosition(4, 1), wxDefaultSpan, wxEXPAND);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Pakage Name:")),
    wxGBPosition(1, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_PackageName = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_PackageName, wxGBPosition(1, 3), wxDefaultSpan, wxEXPAND);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Path in the Pakage:")),
    wxGBPosition(2, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_PathInPackage = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_PathInPackage, wxGBPosition(2, 3), wxDefaultSpan, wxEXPAND);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Package Hash:")),
    wxGBPosition(3, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_PackageHash = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_PackageHash, wxGBPosition(3, 3), wxDefaultSpan, wxEXPAND);

  gbSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Package Info:")),
    wxGBPosition(4, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_PackageInfo = new wxTextCtrl(
    this,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY);
  gbSizer->Add(m_PackageInfo, wxGBPosition(4, 3), wxDefaultSpan, wxEXPAND);

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
  gbSizer->Add(buttonSizer, wxGBPosition(5, 0), wxGBSpan(1, 4), wxALL);

  gbSizer->AddGrowableCol(1, 1);
  gbSizer->AddGrowableCol(3, 1);
  gbSizer->AddGrowableRow(0, 1);
  topSizer->Add(gbSizer, 1, wxEXPAND | wxALL, 5);

  SetSizerAndFit(topSizer);
}

bool GOSettingsOrgans::TransferDataToWindow() {
  m_config.AddOrgansFromArchives();
  for (unsigned i = 0; i < m_OrigOrganList.size(); i++) {
    GOOrgan *o = m_OrigOrganList[i];
    wxString title = o->GetChurchName();

    if (!o->IsUsable(m_config))
      title = _("MISSING - ") + title;
    m_Organs->InsertItem(i, title);
    m_Organs->SetItemPtrData(i, (wxUIntPtr)o);
    m_Organs->SetItem(
      i, 1, o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No"));
    m_Organs->SetItem(
      i,
      2,
      o->GetArchiveID().IsEmpty() ? o->GetODFPath() : o->GetArchivePath());
  }
  m_OrganUp->Disable();
  m_OrganDown->Disable();
  m_OrganTop->Disable();
  m_OrganDel->Disable();
  m_OrganProperties->Disable();
  m_PackageList.clear();
  for (GOArchiveFile *pOrigPackage : m_config.GetArchiveList())
    m_PackageList.push_back(pOrigPackage);
  return true;
}

void GOSettingsOrgans::OnOrganFocused(wxListEvent &event) {
  const int currOrganIndex = event.GetIndex();

  if (currOrganIndex >= 0) {
    const GOOrgan *o = (GOOrgan *)m_Organs->GetItemData(currOrganIndex);
    const bool isPackage = !o->GetArchiveID().IsEmpty();
    const GOArchiveFile *a
      = isPackage ? m_config.GetArchiveByPath(o->GetArchivePath()) : NULL;
    wxString archiveInfo = EMPTY_STRING;

    if (a) {
      if (!a->IsUsable(m_config))
        archiveInfo = _("MISSING - ");
      else if (!a->IsComplete(m_config)) {
        archiveInfo = _("INCOMPLETE - ");

        for (unsigned i = 0; i < a->GetDependencies().size(); i++)
          if (!m_config.GetArchiveByID(a->GetDependencies()[i], true))
            archiveInfo += wxString::Format(
              _("requires '%s' "), a->GetDependencyTitles()[i]);
      }
    }

    m_Builder->ChangeValue(o->GetOrganBuilder());
    m_Recording->ChangeValue(o->GetRecordingDetail());
    m_OrganHash->ChangeValue(o->GetOrganHash());
    m_PackageId->ChangeValue(o->GetArchiveID());
    m_PackageName->ChangeValue(a ? a->GetName() : EMPTY_STRING);
    m_PathInPackage->ChangeValue(isPackage ? o->GetODFPath() : EMPTY_STRING);
    m_PackageHash->ChangeValue(a ? a->GetFileID() : EMPTY_STRING);
    m_PackageInfo->ChangeValue(archiveInfo);
  }
}

void GOSettingsOrgans::OnOrganSelected(wxListEvent &event) {
  long iFirstSelected = m_Organs->GetFirstSelected();

  // find the last selected organ
  long iLastSelected = -1;

  for (long i = iFirstSelected; i >= 0; i = m_Organs->GetNextSelected(i))
    iLastSelected = i;

  if (iFirstSelected >= 0)
    m_OrganDel->Enable();
  else
    m_OrganDel->Disable();

  m_OrganProperties->Enable();
  if (iFirstSelected <= 0) {
    m_OrganTop->Disable();
    m_OrganUp->Disable();
  } else {
    m_OrganTop->Enable();
    m_OrganUp->Enable();
  }
  if (iLastSelected >= 0 && iLastSelected + 1 < m_Organs->GetItemCount())
    m_OrganDown->Enable();
  else
    m_OrganDown->Disable();
}

GOSettingsOrgans::OrganRecs GOSettingsOrgans::GetCurrentOrganRecs() {
  OrganRecs recs;
  const long iFocused = m_Organs->GetFocusedItem();

  for (long l = m_Organs->GetItemCount(), i = 0; i < l; i++)
    recs.push_back(
      {(const GOOrgan *)m_Organs->GetItemData(i), false, i == iFocused});
  for (long i = m_Organs->GetFirstSelected(); i >= 0;
       i = m_Organs->GetNextSelected(i))
    recs[i].is_selected = true;
  return recs;
}

using OrganToLongMap = std::unordered_map<const GOOrgan *, long>;

int wxCALLBACK
organ_ord_compare_callback(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
  const OrganToLongMap *ord = (const OrganToLongMap *)sortData;
  long i1 = ord->at((const GOOrgan *)item1);
  long i2 = ord->at((const GOOrgan *)item2);

  return i1 < i2 ? -1 : i1 > i2 ? 1 : 0;
}

void GOSettingsOrgans::ReorderOrgans(const OrganRecs &newSortedRecs) {
  const int l = newSortedRecs.size();
  OrganToLongMap newOrd;

  for (int i = 0; i < l; i++)
    newOrd[newSortedRecs[i].p_organ] = i;
  m_Organs->SortItems(organ_ord_compare_callback, (wxIntPtr)&newOrd);

  // restore selection and the focus
  for (int i = 0; i < l; i++) {
    const OrganRec &rec = newSortedRecs[i];

    m_Organs->Select(i, rec.is_selected);
    if (rec.is_focused)
      m_Organs->Focus(i);
  }
}

void GOSettingsOrgans::OnOrganUp(wxCommandEvent &event) {
  OrganRecs recs(GetCurrentOrganRecs());
  const int l = recs.size();

  if (l && !recs[0].is_selected)
    for (int i = 1; i < l; i++) {
      OrganRec &rec = recs[i];

      if (rec.is_selected)
        std::swap(rec, recs[i - 1]);
    }
  ReorderOrgans(recs);
}

void GOSettingsOrgans::OnOrganDown(wxCommandEvent &event) {
  OrganRecs recs(GetCurrentOrganRecs());
  const int l = recs.size();

  if (l && !recs[l - 1].is_selected)
    for (int i = l - 2; i >= 0; i--) {
      OrganRec &rec = recs[i];

      if (rec.is_selected)
        std::swap(rec, recs[i + 1]);
    }
  ReorderOrgans(recs);
}

void GOSettingsOrgans::OnOrganTop(wxCommandEvent &event) {
  OrganRecs oldRecs(GetCurrentOrganRecs());
  OrganRecs newRecs;

  // add all selected to the new order
  std::copy_if(
    oldRecs.begin(),
    oldRecs.end(),
    std::back_inserter(newRecs),
    [](OrganRec &rec) { return rec.is_selected; });

  // add all unselected to the new order
  std::copy_if(
    oldRecs.begin(),
    oldRecs.end(),
    std::back_inserter(newRecs),
    [](OrganRec &rec) { return !rec.is_selected; });
  ReorderOrgans(newRecs);
}

static wxString NAMES_DELIM = ", ";

void GOSettingsOrgans::OnOrganDel(wxCommandEvent &event) {
  std::set<long> selectedItemSet;
  std::vector<long> selectedItems;
  wxString organNames;

  for (long i = m_Organs->GetFirstSelected(); i >= 0;
       i = m_Organs->GetNextSelected(i)) {
    selectedItems.push_back(i);
    selectedItemSet.insert(i);
    organNames += (organNames.IsEmpty() ? EMPTY_STRING : NAMES_DELIM)
      + m_Organs->GetItemText(i);
  }

  if (selectedItems.size()) {
    // find all package paths are still used by the remaining organs
    std::set<wxString> packagePathsUsed;

    for (long l = m_Organs->GetItemCount(), i = 0; i < l; i++)
      if (selectedItemSet.find(i) == selectedItemSet.end()) { // not selected
        const wxString &pkgPath
          = ((GOOrgan *)m_Organs->GetItemData(i))->GetArchivePath();

        if (!pkgPath.IsEmpty())
          packagePathsUsed.insert(pkgPath);
      }

    // find all package ids that are still used
    std::set<wxString> packageIdsUsed;

    for (const GOArchiveFile *pA : m_PackageList)
      if (packagePathsUsed.find(pA->GetPath()) != packagePathsUsed.end())
        for (const wxString &depId : pA->GetDependencies())
          packageIdsUsed.insert(depId);

    // find all packages that are not used anymore and will be removed
    std::set<GOArchiveFile *> packagesToDelete;
    wxString packageNamesToDelete;

    for (GOArchiveFile *pA : m_PackageList)
      if (
        packagePathsUsed.find(pA->GetPath()) == packagePathsUsed.end()
        && packageIdsUsed.find(pA->GetID()) == packageIdsUsed.end()) {
        packagesToDelete.insert(pA);
        packageNamesToDelete
          += (packageNamesToDelete.IsEmpty() ? EMPTY_STRING : NAMES_DELIM)
          + pA->GetName();
      }

    wxString addDelMessage = packageNamesToDelete.IsEmpty()
      ? EMPTY_STRING
      : wxString::Format(
        _("\n(The following packages will also be deleted: %s)"),
        packageNamesToDelete);

    if (
      wxMessageBox(
        wxString::Format(
          _("Do you want to remove %s%s?"), organNames, addDelMessage),
        _("Organs"),
        wxYES_NO | wxICON_EXCLAMATION,
        this)
      == wxYES) {
      // do actual deleting

      for (int i = selectedItems.size() - 1; i >= 0; i--)
        m_Organs->DeleteItem(selectedItems[i]);

      m_PackageList.erase(
        std::remove_if(
          m_PackageList.begin(),
          m_PackageList.end(),
          [&](GOArchiveFile *pA) {
            return packagesToDelete.find(pA) != packagesToDelete.end();
          }),
        m_PackageList.end());

      m_OrganUp->Disable();
      m_OrganDown->Disable();
      m_OrganTop->Disable();
      m_OrganDel->Disable();
      m_OrganProperties->Disable();
    }
  }
}

void GOSettingsOrgans::OnOrganProperties(wxCommandEvent &event) {
  GOOrgan *o = (GOOrgan *)m_Organs->GetItemData(m_Organs->GetFirstSelected());
  GOMidiEventDialog dlg(
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

bool GOSettingsOrgans::TransferDataFromWindow() {
  for (unsigned i = 0; i < m_OrigOrganList.size(); i++) {
    bool found = false;
    for (long j = 0; j < m_Organs->GetItemCount(); j++)
      if (m_Organs->GetItemData(j) == (wxUIntPtr)m_OrigOrganList[i])
        found = true;
    if (!found)
      delete m_OrigOrganList[i];
    m_OrigOrganList[i] = NULL;
  }
  m_OrigOrganList.clear();
  for (long i = 0; i < m_Organs->GetItemCount(); i++)
    m_OrigOrganList.push_back((GOOrgan *)m_Organs->GetItemData(i));

  ptr_vector<GOArchiveFile> &origPackageList = m_config.GetArchiveList();

  for (unsigned n = origPackageList.size(), i = 0; i < n; i++) {
    GOArchiveFile *&origPackage = origPackageList[i];

    if (
      std::find(m_PackageList.begin(), m_PackageList.end(), origPackage)
      == m_PackageList.end()) {
      delete origPackage;
      origPackage = NULL;
    }
  }
  origPackageList.clear();
  for (GOArchiveFile *newArch : m_PackageList)
    origPackageList.push_back(newArch);
  std::cout << "GOSettingsOrgans::TransferDataFromWindow origPackageList.size="
            << origPackageList.size() << std::endl;
  return true;
}
