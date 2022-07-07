/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsOrgans.h"

#include <algorithm>
#include <set>

#include <wx/button.h>
#include <wx/dir.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "GOOrgan.h"
#include "archive/GOArchiveFile.h"
#include "config/GOConfig.h"
#include "dialogs/midi-event/GOMidiEventDialog.h"
#include "files/GOStdFileName.h"

#include "GODefinitionFile.h"

static wxString EMPTY_STRING = wxEmptyString;

BEGIN_EVENT_TABLE(GOSettingsOrgans, wxPanel)
EVT_LIST_ITEM_FOCUSED(ID_ORGANS, GOSettingsOrgans::OnOrganFocused)
EVT_LIST_ITEM_SELECTED(ID_ORGANS, GOSettingsOrgans::OnOrganSelected)
EVT_LIST_ITEM_DESELECTED(ID_ORGANS, GOSettingsOrgans::OnOrganSelected)
EVT_BUTTON(ID_ORGAN_UP, GOSettingsOrgans::OnOrganUp)
EVT_BUTTON(ID_ORGAN_DOWN, GOSettingsOrgans::OnOrganDown)
EVT_BUTTON(ID_ORGAN_TOP, GOSettingsOrgans::OnOrganTop)
EVT_BUTTON(ID_ORGAN_DEL, GOSettingsOrgans::OnOrganDel)
EVT_BUTTON(ID_ORGAN_MIDI, GOSettingsOrgans::OnOrganMidi)
EVT_BUTTON(ID_ORGAN_RELOCATE, GOSettingsOrgans::OnOrganRelocate)
EVT_BUTTON(ID_DEL_CACHE, GOSettingsOrgans::OnDelCache)
EVT_BUTTON(ID_DEL_PRESET, GOSettingsOrgans::OnDelPreset)
END_EVENT_TABLE()

GOSettingsOrgans::GOSettingsOrgans(
  GOConfig &settings, GOMidi &midi, wxWindow *parent)
  : wxPanel(parent, wxID_ANY),
    m_config(settings),
    m_midi(midi),
    m_OrigOrganList(settings.GetOrganList()),
    m_OrigPackageList(settings.GetArchiveList()) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);
  wxGridBagSizer *const gbSizer = new wxGridBagSizer(5, 5);

  m_Organs = new wxListView(
    this,
    ID_ORGANS,
    wxDefaultPosition,
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
    new wxStaticText(this, wxID_ANY, _("Organ hash:")),
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
    new wxStaticText(this, wxID_ANY, _("Package id:")),
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
    new wxStaticText(this, wxID_ANY, _("Package name:")),
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
    new wxStaticText(this, wxID_ANY, _("Path in package:")),
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
    new wxStaticText(this, wxID_ANY, _("Package hash:")),
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
    new wxStaticText(this, wxID_ANY, _("Package info:")),
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
  m_OrganDown = new wxButton(this, ID_ORGAN_DOWN, _("&Down"));
  m_OrganUp = new wxButton(this, ID_ORGAN_UP, _("&Up"));
  m_OrganTop = new wxButton(this, ID_ORGAN_TOP, _("&Top"));
  m_OrganMidi = new wxButton(this, ID_ORGAN_MIDI, _("&MIDI..."));
  m_OrganRelocate = new wxButton(this, ID_ORGAN_RELOCATE, _("&Relocate..."));
  buttonSizer->Add(m_OrganDown, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganUp, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganTop, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganMidi, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganRelocate, 0, wxALIGN_LEFT | wxALL, 5);
  gbSizer->Add(buttonSizer, wxGBPosition(5, 0), wxGBSpan(1, 4), wxALL);

  buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_DelCache = new wxButton(this, ID_DEL_CACHE, _("Delete &cache(s)"));
  m_DelPreset = new wxButton(this, ID_DEL_PRESET, _("Delete &preset(s)"));
  m_OrganDel = new wxButton(this, ID_ORGAN_DEL, _("&Delete"));
  buttonSizer->Add(m_DelCache, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_DelPreset, 0, wxALIGN_LEFT | wxALL, 5);
  buttonSizer->Add(m_OrganDel, 0, wxALIGN_LEFT | wxALL, 5);
  gbSizer->Add(buttonSizer, wxGBPosition(6, 0), wxGBSpan(1, 4), wxALL);

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
  m_PackagesByPath.clear();
  for (GOArchiveFile *pPackage : m_OrigPackageList)
    m_PackagesByPath[pPackage->GetPath()] = pPackage;
  RefreshButtons();
  return true;
}

GOArchiveFile *GOSettingsOrgans::GetPkgByPath(const wxString &path) const {
  const auto iter = m_PackagesByPath.find(path);

  return iter == m_PackagesByPath.end() ? NULL : iter->second;
}

void GOSettingsOrgans::RefreshFocused() {
  const int currOrganIndex = m_Organs->GetFocusedItem();

  if (currOrganIndex >= 0) {
    const GOOrgan *o = (GOOrgan *)m_Organs->GetItemData(currOrganIndex);
    const bool isPackage = !o->GetArchiveID().IsEmpty();
    const GOArchiveFile *a
      = isPackage ? GetPkgByPath(o->GetArchivePath()) : NULL;
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

void GOSettingsOrgans::OnOrganFocused(wxListEvent &event) {
  RefreshFocused();
  RefreshButtons();
}

void GOSettingsOrgans::OnOrganSelected(wxListEvent &event) { RefreshButtons(); }

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

void GOSettingsOrgans::RefreshButtons() {
  if (m_Organs->GetFocusedItem() >= 0) {
    m_OrganMidi->Enable();
  } else {
    m_OrganMidi->Disable();
  }

  long iFirstSelected = m_Organs->GetFirstSelected();

  // find the last selected organ
  long iLastSelected = -1;

  for (long i = iFirstSelected; i >= 0; i = m_Organs->GetNextSelected(i))
    iLastSelected = i;

  if (iFirstSelected >= 0) {
    m_OrganDel->Enable();
    m_DelCache->Enable();
    m_DelPreset->Enable();
  } else {
    m_OrganDel->Disable();
    m_DelCache->Disable();
    m_DelPreset->Disable();
  }

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
      if (selectedItemSet.find(i) == selectedItemSet.end()) {
        // not selected
        const wxString &pkgPath
          = ((GOOrgan *)m_Organs->GetItemData(i))->GetArchivePath();

        if (!pkgPath.IsEmpty())
          packagePathsUsed.insert(pkgPath);
      }

    // find all package ids that are still used
    std::set<wxString> packageIdsUsed;

    for (const auto &pP : m_PackagesByPath)
      if (packagePathsUsed.find(pP.first) != packagePathsUsed.end())
        for (const wxString &depId : pP.second->GetDependencies())
          packageIdsUsed.insert(depId);

    // find all packages that are not used anymore and will be removed
    std::set<GOArchiveFile *> packagesToDelete;
    wxString packageNamesToDelete;

    for (const auto &pP : m_PackagesByPath)
      if (
        packagePathsUsed.find(pP.first) == packagePathsUsed.end()
        && packageIdsUsed.find(pP.second->GetID()) == packageIdsUsed.end()) {
        packagesToDelete.insert(pP.second);
        packageNamesToDelete
          += (packageNamesToDelete.IsEmpty() ? EMPTY_STRING : NAMES_DELIM)
          + pP.second->GetName();
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

      for (int i = selectedItems.size() - 1; i >= 0; i--) {
        const long j = selectedItems[i];
        GOOrgan *pOrgan = (GOOrgan *)m_Organs->GetItemData(j);

        m_Organs->DeleteItem(j);
        if (
          std::find(m_OrigOrganList.begin(), m_OrigOrganList.end(), pOrgan)
          == m_OrigOrganList.end())
          // this organ has been relocated from the original path. Delete it
          delete pOrgan;
        // otherwise it will be deleted on OK and want be deleted on Cancel
      }

      // delete the packages that are not existing among the original packages
      for (GOArchiveFile *pA : packagesToDelete) {
        if (
          std::find(m_OrigPackageList.begin(), m_OrigPackageList.end(), pA)
          == m_OrigPackageList.end()) {
          delete pA;
        }
        m_PackagesByPath.erase(pA->GetPath());
      }

      RefreshButtons();
    }
  }
}

void GOSettingsOrgans::OnOrganMidi(wxCommandEvent &event) {
  const int currOrganIndex = m_Organs->GetFocusedItem();

  if (currOrganIndex >= 0) {
    GOOrgan *o = (GOOrgan *)m_Organs->GetItemData(currOrganIndex);
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
      currOrganIndex,
      1,
      o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No"));
  }
}

void GOSettingsOrgans::ReplaceOrganPath(
  const long index, const wxString &newPath) {
  GOOrgan *pOrgan = (GOOrgan *)m_Organs->GetItemData(index);
  auto oldHashEntry = m_OldHashes.find(pOrgan);
  const wxString oldHash = oldHashEntry == m_OldHashes.end()
    ? pOrgan->GetOrganHash()
    // the first relocating of this organ
    : oldHashEntry->second;
  // Otherwise this organ has already been relocated. Use the
  // original hash

  GOOrgan *pNewOrgan = new GOOrgan(*pOrgan);

  if (pOrgan->GetArchiveID().IsEmpty())
    pNewOrgan->SetODFPath(newPath);
  else
    pNewOrgan->SetArchivePath(newPath);
  m_Organs->SetItemPtrData(index, (wxUIntPtr)pNewOrgan);
  m_Organs->SetItem(index, 2, newPath);

  // save data for renaming cache and presets on TransferDataFromWindow
  if (pNewOrgan->GetOrganHash() != oldHash)
    m_OldHashes.insert({pNewOrgan, oldHash});

  // delete pOrgan if it was created during a previous relocate
  if (
    std::find(m_OrigOrganList.begin(), m_OrigOrganList.end(), pOrgan)
    == m_OrigOrganList.end())
    delete pOrgan;
}

void GOSettingsOrgans::OnOrganRelocate(wxCommandEvent &event) {
  const int currOrganIndex = m_Organs->GetFocusedItem();

  if (currOrganIndex >= 0) {
    GOOrgan *pOrgan = (GOOrgan *)m_Organs->GetItemData(currOrganIndex);
    const bool isArchive = !pOrgan->GetArchiveID().IsEmpty();
    const wxString oldPath
      = isArchive ? pOrgan->GetArchivePath() : pOrgan->GetODFPath();
    const wxFileName oldFileName(oldPath);
    const wxString &wildcard = isArchive
      ? GOStdFileName::getPackageDlgWildcard()
      : GOStdFileName::getOdfDlgWildcard();

    // select new filename
    wxFileDialog dlg(
      this,
      _("Relocate organ path"),
      oldFileName.GetPath(),
      oldFileName.GetFullName(),
      wildcard,
      wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
      const wxString newPath = dlg.GetPath();

      if (newPath != oldPath) {
        if (isArchive) {
          GOArchiveFile *pOldPkg = GetPkgByPath(oldPath);
          GOArchiveFile *pNewPkg = GetPkgByPath(newPath);

          m_PackagesByPath.erase(oldPath);

          if (!pNewPkg && pOldPkg) {
            // the package with the new path is not on file yet. Create and
            // replace it
            pNewPkg = new GOArchiveFile(*pOldPkg);

            pNewPkg->SetPath(newPath);
            m_PackagesByPath[newPath] = pNewPkg;
          }

          // delete pOldPkg if it has been created in this dialog
          if (
            std::find(
              m_OrigPackageList.begin(), m_OrigPackageList.end(), pOldPkg)
            == m_OrigPackageList.end())
            delete pOldPkg;

          // switch other organs from the same package to the new archive
          for (long l = m_Organs->GetItemCount(), i = 0; i < l; i++) {
            GOOrgan *pO = (GOOrgan *)m_Organs->GetItemData(i);

            if (pO->GetArchivePath() == oldPath)
              ReplaceOrganPath(i, newPath);
          }
        } else {
          ReplaceOrganPath(currOrganIndex, newPath);
          RefreshFocused();
        }
      }
    }
  }
}

void delete_files_by_pattern(const wxString &dirPath, const wxString &pattern) {
  wxDir dir(dirPath);

  if (dir.IsOpened()) {
    const wxString fileNamePrefix = dirPath + wxFileName::GetPathSeparator();
    wxString fileName;

    for (bool exists = dir.GetFirst(&fileName, pattern); exists;
         exists = dir.GetNext(&fileName)) {
      wxString fullName = fileNamePrefix + fileName;

      if (wxRemoveFile(fullName))
        wxLogInfo(_("Delete file %s"), fullName);
      else
        wxLogError(_("Unable to delete file %s"), fullName);
    }
  }
}

void rename_files_by_pattern(
  const wxString &dirPath,
  const wxString &suffixPattern,
  const wxString &oldPrefix,
  const wxString &newPrefix) {
  wxDir dir(dirPath);

  if (dir.IsOpened()) {
    const size_t oldPrefixLen = oldPrefix.Length();
    const wxString fileNamePrefix = dirPath + wxFileName::GetPathSeparator();
    const wxString pattern = oldPrefix + suffixPattern;
    wxString fileName;

    for (bool exists = dir.GetFirst(&fileName, pattern); exists;
         exists = dir.GetNext(&fileName)) {
      wxString oldName = fileNamePrefix + fileName;
      wxString newName
        = fileNamePrefix + newPrefix + fileName.Mid(oldPrefixLen);

      if (wxRenameFile(oldName, newName, false))
        wxLogInfo(_("Renamed file %s to %s"), oldName, newName);
      else
        wxLogError(_("Unable to rename file %s to %s"), oldName, newName);
    }
  }
}

void GOSettingsOrgans::DeleteCache(const GOOrgan *pOrgan) {
  const wxString organHash = pOrgan->GetOrganHash();

  if (!organHash.IsEmpty())
    delete_files_by_pattern(
      m_config.OrganCachePath(),
      GODefinitionFile::GetCacheFilePattern(organHash));
}

void GOSettingsOrgans::DeletePresets(const GOOrgan *pOrgan, bool toAsk) {
  const wxString organHash = pOrgan->GetOrganHash();

  if (
    ! organHash.IsEmpty() && (
      ! toAsk
      || wxMessageBox(
        wxString::Format(
          _("Do you want to delete all presets for the organ %s?"), pOrgan->GetChurchName()),
	_("Delete organ settings"),
	wxYES_NO | wxICON_EXCLAMATION,
	this
      ) == wxYES
    )
  )
    delete_files_by_pattern(
      m_config.OrganSettingsPath(),
      GODefinitionFile::GetSettingFilePattern(organHash));
}

void GOSettingsOrgans::OnDelCache(wxCommandEvent &event) {
  for (long i = m_Organs->GetFirstSelected(); i >= 0;
       i = m_Organs->GetNextSelected(i)) {
    const GOOrgan *pOrgan = (const GOOrgan *)m_Organs->GetItemData(i);

    if (pOrgan)
      DeleteCache(pOrgan);
  }
}

void GOSettingsOrgans::OnDelPreset(wxCommandEvent &event) {
  for (long i = m_Organs->GetFirstSelected(); i >= 0;
       i = m_Organs->GetNextSelected(i)) {
    const GOOrgan *pOrgan = (const GOOrgan *)m_Organs->GetItemData(i);

    if (pOrgan)
      DeletePresets(pOrgan, true);
  }
}

bool GOSettingsOrgans::TransferDataFromWindow() {
  std::set<const GOOrgan *> newOrgans;
  std::unordered_map<wxString, wxString, wxStringHash, wxStringEqual>
    hashesToRename;

  for (long i = 0; i < m_Organs->GetItemCount(); i++) {
    const GOOrgan *pOrgan = (const GOOrgan *)m_Organs->GetItemData(i);
    auto itOldHash = m_OldHashes.find(pOrgan);

    newOrgans.insert(pOrgan);
    if (itOldHash != m_OldHashes.end())
      hashesToRename[itOldHash->second] = pOrgan->GetOrganHash();
  }

  for (unsigned i = 0; i < m_OrigOrganList.size(); i++) {
    GOOrgan *&pOrgan = m_OrigOrganList[i];

    if (newOrgans.find(pOrgan) == newOrgans.end()) {
      // actually delete the organ
      auto itHashToRename = hashesToRename.find(pOrgan->GetOrganHash());

      if (itHashToRename == hashesToRename.end()) {
        DeleteCache(pOrgan);
        DeletePresets(pOrgan, false);
      } else {
        rename_files_by_pattern(
          m_config.OrganCachePath(),
          GODefinitionFile::GetCacheFilePattern(wxEmptyString),
          itHashToRename->first,
          itHashToRename->second);
        rename_files_by_pattern(
          m_config.OrganSettingsPath(),
          GODefinitionFile::GetSettingFilePattern(wxEmptyString),
          itHashToRename->first,
          itHashToRename->second);
      }
      delete pOrgan;
    }
    pOrgan = NULL;
  }
  m_OrigOrganList.clear();
  for (long i = 0; i < m_Organs->GetItemCount(); i++) {
    GOOrgan *pOrgan = (GOOrgan *)m_Organs->GetItemData(i);

    m_OrigOrganList.push_back(pOrgan);
  }

  for (unsigned n = m_OrigPackageList.size(), i = 0; i < n; i++) {
    GOArchiveFile *&origPackage = m_OrigPackageList[i];

    if (!GetPkgByPath(origPackage->GetPath()))
      delete origPackage;
    // for preventing double deleting during clear()
    origPackage = NULL;
  }
  m_OrigPackageList.clear();
  for (const auto &it : m_PackagesByPath)
    m_OrigPackageList.push_back(it.second);
  return true;
}
