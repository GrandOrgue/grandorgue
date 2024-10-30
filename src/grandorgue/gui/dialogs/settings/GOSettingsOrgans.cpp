/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsOrgans.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <set>
#include <unordered_set>

#include <wx/button.h>
#include <wx/dir.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/gbsizer.h>
#include <wx/grid.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "archive/GOArchiveFile.h"
#include "archive/GOArchiveIndex.h"
#include "config/GOConfig.h"
#include "dialogs/midi-event/GOMidiEventDialog.h"
#include "files/GOStdFileName.h"
#include "size/GOAdditionalSizeKeeperProxy.h"
#include "wxcontrols/GOGrid.h"

#include "GOOrgan.h"
#include "GOOrganController.h"

static const wxString EMPTY_STRING = wxEmptyString;

BEGIN_EVENT_TABLE(GOSettingsOrgans, wxPanel)
EVT_GRID_CMD_SELECT_CELL(ID_ORGANS, GOSettingsOrgans::OnOrganSelectCell)
EVT_GRID_CMD_RANGE_SELECT(ID_ORGANS, GOSettingsOrgans::OnOrganRangeSelect)
EVT_CHAR_HOOK(GOSettingsOrgans::OnCharHook)
EVT_BUTTON(ID_ORGAN_UP, GOSettingsOrgans::OnOrganUp)
EVT_BUTTON(ID_ORGAN_DOWN, GOSettingsOrgans::OnOrganDown)
EVT_BUTTON(ID_ORGAN_TOP, GOSettingsOrgans::OnOrganTop)
EVT_BUTTON(ID_ORGAN_DEL, GOSettingsOrgans::OnOrganDel)
EVT_BUTTON(ID_ORGAN_MIDI, GOSettingsOrgans::OnOrganMidi)
EVT_BUTTON(ID_ORGAN_RELOCATE, GOSettingsOrgans::OnOrganRelocate)
EVT_BUTTON(ID_DEL_CACHE, GOSettingsOrgans::OnDelCache)
EVT_BUTTON(ID_DEL_PRESET, GOSettingsOrgans::OnDelPreset)
END_EVENT_TABLE()

enum { GRID_COL_NAME = 0, GRID_COL_MIDI, GRID_COL_PATH, GRID_N_COLS };

GOSettingsOrgans::GOSettingsOrgans(
  GOConfig &settings,
  GOMidi &midi,
  GOTabbedDialog *pDlg,
  const wxString &name,
  const wxString &label)
  : GODialogTab(pDlg, name, label),
    m_config(settings),
    m_midi(midi),
    m_OrigOrganList(settings.GetOrganList()),
    m_OrigPackageList(settings.GetArchiveList()) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);
  wxGridBagSizer *const gbSizer = new wxGridBagSizer(5, 5);

  m_GridOrgans
    = new GOGrid(this, ID_ORGANS, wxDefaultPosition, wxSize(100, 40));
  m_GridOrgans->CreateGrid(0, GRID_N_COLS, wxGrid::wxGridSelectRows);
  m_GridOrgans->HideRowLabels();
  m_GridOrgans->EnableEditing(false);
  m_GridOrgans->SetColSize(GRID_COL_NAME, 200);
  m_GridOrgans->SetColLabelValue(GRID_COL_NAME, _("Name"));
  m_GridOrgans->SetColSize(GRID_COL_MIDI, 50);
  m_GridOrgans->SetColLabelValue(GRID_COL_MIDI, _("MIDI"));
  m_GridOrgans->SetColSize(GRID_COL_PATH, 300);
  m_GridOrgans->SetColLabelValue(GRID_COL_PATH, _("Path"));
  m_GridOrgans->SetColumnRightVisible(GRID_COL_PATH, true);

  gbSizer->Add(
    m_GridOrgans, wxGBPosition(0, 0), wxGBSpan(1, 4), wxALL | wxEXPAND, 5);

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

using StringSet = std::unordered_set<wxString, wxStringHash, wxStringEqual>;

static StringSet collect_hashes_from_organ_files(
  const wxString &dirPath, const wxString &pattern) {
  StringSet hashSet;
  wxArrayString files;

  wxDir::GetAllFiles(dirPath, &files, pattern);

  for (const wxString &fileName : files)
    hashSet.insert(GOStdFileName::extractOrganHash(fileName));
  return hashSet;
}

GOSettingsOrgans::PackageSlotSet GOSettingsOrgans::GetUsedPackages(
  std::unordered_set<unsigned> itemsToExtract) {
  // fill up a map pkgId -> pkgSlot
  std::unordered_map<
    const wxString,
    const PackageSlot *,
    wxStringHash,
    wxStringEqual>
    pkgSlotById;

  for (const PackageSlot &pkgSlot : m_PackageSlots)
    if (pkgSlot.is_present)
      pkgSlotById[pkgSlot.p_CurrentPkg->GetID()] = &pkgSlot;

  // find all package paths are still used by the remaining organs
  PackageSlotSet packagesUsed;

  for (unsigned l = m_GridOrgans->GetNumberRows(), i = 0; i < l; i++)
    if (itemsToExtract.find(i) == itemsToExtract.end()) { // not selected
      const PackageSlot *pPkgSlot = m_OrganSlotPtrsByGridLine[i]->p_PackageSlot;

      if (pPkgSlot)
        packagesUsed.insert(pPkgSlot);
    }

  // add all packages referenced by another packages
  std::deque<const PackageSlot *> slotsToLook(
    packagesUsed.begin(), packagesUsed.end());

  while (!slotsToLook.empty()) {
    const PackageSlot *pPkgSlot = slotsToLook.front();

    slotsToLook.pop_front();
    for (const wxString &depId : pPkgSlot->p_CurrentPkg->GetDependencies()) {
      auto dep = pkgSlotById.find(depId);

      if (dep != pkgSlotById.end()) {
        const PackageSlot *pDepSlot = dep->second;

        if (packagesUsed.find(pDepSlot) == packagesUsed.end()) {
          // pDepSlot - a new package; it is not currently used.
          packagesUsed.insert(pDepSlot);   // add to used packages
          slotsToLook.push_back(pDepSlot); // look at it's dependencies
        }
      }
    }
  }
  return packagesUsed;
}

void GOSettingsOrgans::DisplayMidiCell(unsigned rowN, GOOrgan *pOrgan) {
  m_GridOrgans->SetCellValue(
    rowN,
    GRID_COL_MIDI,
    pOrgan->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No"));
}

void GOSettingsOrgans::DisplayPathCell(unsigned rowN, const wxString &path) {
  m_GridOrgans->SetCellValue(rowN, GRID_COL_PATH, path);
}

void GOSettingsOrgans::FillGridRow(unsigned rowN, OrganSlot &organSlot) {
  GOOrgan *o = organSlot.p_CurrentOrgan;
  wxString title = o->GetChurchName();

  if (!o->IsUsable(m_config))
    title = _("MISSING - ") + title;

  // m_OrganSlotPtrsByGridLine has at least rowN+1 elements
  for (unsigned i = m_OrganSlotPtrsByGridLine.size(); i <= rowN; i++)
    m_OrganSlotPtrsByGridLine.push_back(nullptr);

  m_OrganSlotPtrsByGridLine[rowN] = &organSlot;

  m_GridOrgans->SetCellValue(rowN, GRID_COL_NAME, title);
  DisplayMidiCell(rowN, o);
  DisplayPathCell(rowN, organSlot.m_CurrentPath);
}

bool GOSettingsOrgans::TransferDataToWindow() {
  StringSet organHashesWithCache = collect_hashes_from_organ_files(
    m_config.OrganCachePath(), GOStdFileName::composeCacheFilePattern());
  StringSet organHashesWithPresets = collect_hashes_from_organ_files(
    m_config.OrganSettingsPath(), GOStdFileName::composeSettingFilePattern());

  m_PackageSlots.clear();
  m_PackageSlots.reserve(m_OrigPackageList.size());
  m_PackageSlotByPath.clear();
  for (unsigned i = 0; i < m_OrigPackageList.size(); i++) {
    GOArchiveFile *pPackage = m_OrigPackageList[i];

    m_PackageSlots.emplace_back(pPackage);
    m_PackageSlotByPath[pPackage->GetPath()] = &m_PackageSlots[i];
  }

  const unsigned nOrgans = m_OrigOrganList.size();

  m_OrganSlots.clear();
  m_OrganSlots.reserve(nOrgans);
  m_OrganSlotPtrsByGridLine.reserve(nOrgans);

  m_GridOrgans->ClearGrid();
  m_GridOrgans->AppendRows(nOrgans);

  for (unsigned i = 0; i < nOrgans; i++) {
    m_OrganSlots.emplace_back();

    OrganSlot &organSlot = m_OrganSlots[i];
    GOOrgan *o = m_OrigOrganList[i];
    wxString hash = o->GetOrganHash();

    organSlot.p_OrigOrgan = o;
    organSlot.p_CurrentOrgan = o;
    organSlot.is_packaged = !o->GetArchiveID().IsEmpty();
    organSlot.m_CurrentPath
      = organSlot.is_packaged ? o->GetArchivePath() : o->GetODFPath();
    organSlot.is_AnyCacheExisting
      = organHashesWithCache.find(hash) != organHashesWithCache.end();
    organSlot.is_AnyPresetExisting
      = organHashesWithPresets.find(hash) != organHashesWithPresets.end();

    if (organSlot.is_packaged) {
      auto packageIter = m_PackageSlotByPath.find(o->GetArchivePath());

      organSlot.p_PackageSlot
        = packageIter != m_PackageSlotByPath.end() ? packageIter->second : NULL;
    } else
      organSlot.p_PackageSlot = NULL;
    organSlot.is_present = true;

    m_OrganSlotByPath[organSlot.m_CurrentPath] = &organSlot;

    FillGridRow(i, organSlot);
  }
  if (nOrgans >= 0)
    m_GridOrgans->SetGridCursor(0, 0);

  // mark unused packages for deleting
  PackageSlotSet packagesUsed = GetUsedPackages();

  for (PackageSlot &pkgSlot : m_PackageSlots)
    if (
      pkgSlot.is_present && packagesUsed.find(&pkgSlot) == packagesUsed.end()) {
      pkgSlot.is_present = false;
      m_PackageSlotByPath.erase(pkgSlot.p_CurrentPkg->GetPath());
    }

  RefreshButtons();
  return true;
}

void GOSettingsOrgans::RefreshFocused(const int currOrganIndex) {
  const OrganSlot *pSlot = currOrganIndex >= 0
      && currOrganIndex < (int)m_OrganSlotPtrsByGridLine.size()
    ? m_OrganSlotPtrsByGridLine[currOrganIndex]
    : nullptr;
  const GOOrgan *o = pSlot ? pSlot->p_CurrentOrgan : NULL;
  const bool isPackage = pSlot && pSlot->is_packaged;
  const GOArchiveFile *a = isPackage && pSlot->p_PackageSlot
    ? pSlot->p_PackageSlot->p_OrigPkg
    : NULL;
  wxString archiveInfo = EMPTY_STRING;

  if (a) {
    if (!a->IsUsable())
      archiveInfo = _("MISSING - ");
    else if (!a->IsComplete(m_config)) {
      archiveInfo = _("INCOMPLETE - ");

      for (unsigned i = 0; i < a->GetDependencies().size(); i++)
        if (!m_config.GetArchiveByID(a->GetDependencies()[i], true))
          archiveInfo += wxString::Format(
            _("requires '%s' "), a->GetDependencyTitles()[i]);
    }
  }

  m_Builder->ChangeValue(o ? o->GetOrganBuilder() : EMPTY_STRING);
  m_Recording->ChangeValue(o ? o->GetRecordingDetail() : EMPTY_STRING);
  m_OrganHash->ChangeValue(o ? o->GetOrganHash() : EMPTY_STRING);
  m_PackageId->ChangeValue(o ? o->GetArchiveID() : EMPTY_STRING);
  m_PackageName->ChangeValue(a ? a->GetName() : EMPTY_STRING);
  m_PathInPackage->ChangeValue(isPackage && o ? o->GetODFPath() : EMPTY_STRING);
  m_PackageHash->ChangeValue(a ? a->GetArchiveHash() : EMPTY_STRING);
  m_PackageInfo->ChangeValue(archiveInfo);
}

void GOSettingsOrgans::OnCharHook(wxKeyEvent &ev) {
  if (ev.GetKeyCode() == WXK_DELETE && !ev.HasAnyModifiers())
    DelSelectedOrgans();
  else
    ev.Skip(true);
}

void GOSettingsOrgans::OnOrganSelectCell(wxGridEvent &event) {
  RefreshFocused(event.GetRow());
  RefreshButtons();
}

void GOSettingsOrgans::OnOrganRangeSelect(wxGridRangeSelectEvent &event) {
  RefreshButtons();
}

GOSettingsOrgans::VisibleOrganRecs GOSettingsOrgans::GetCurrentOrganRecs() {
  VisibleOrganRecs recs;
  const int iFocused = m_GridOrgans->GetGridCursorRow();

  for (int l = m_GridOrgans->GetNumberRows(), i = 0; i < l; i++)
    recs.push_back({m_OrganSlotPtrsByGridLine[i], false, i == iFocused});
  for (int i : m_GridOrgans->GetSelectedRows())
    recs[i].is_selected = true;
  return recs;
}

void GOSettingsOrgans::RefreshButtons() {
  if (m_GridOrgans->GetGridCursorRow() >= 0) {
    m_OrganMidi->Enable();
  } else {
    m_OrganMidi->Disable();
  }

  int iFirstSelected = -1;
  int iLastSelected = -1;
  bool isAnyCacheExisting = false;
  bool isAnyPresetExisting = false;

  for (int i : m_GridOrgans->GetSelectedRows()) {
    const OrganSlot *pSlot = m_OrganSlotPtrsByGridLine[i];

    if (iFirstSelected < 0)
      iFirstSelected = i;
    iLastSelected = i;
    isAnyCacheExisting = isAnyCacheExisting || pSlot->is_AnyCacheExisting;
    isAnyPresetExisting = isAnyPresetExisting || pSlot->is_AnyPresetExisting;
  }

  if (iFirstSelected <= 0) {
    m_OrganTop->Disable();
    m_OrganUp->Disable();
  } else {
    m_OrganTop->Enable();
    m_OrganUp->Enable();
  }
  if (iLastSelected >= 0 && iLastSelected + 1 < m_GridOrgans->GetNumberRows())
    m_OrganDown->Enable();
  else
    m_OrganDown->Disable();

  if (isAnyCacheExisting)
    m_DelCache->Enable();
  else
    m_DelCache->Disable();
  if (isAnyPresetExisting)
    m_DelPreset->Enable();
  else
    m_DelPreset->Disable();

  if (iFirstSelected >= 0)
    m_OrganDel->Enable();
  else
    m_OrganDel->Disable();
}

void GOSettingsOrgans::ReorderOrgans(const VisibleOrganRecs &newSortedRecs) {
  const int l = newSortedRecs.size();

  for (int i = 0; i < l; i++) {
    const VisibleOrganRec &rec = newSortedRecs[i];

    FillGridRow(i, *rec.p_OrganSlot);

    // restore selection and the focus
    if (rec.is_selected)
      m_GridOrgans->SelectRow(i, true);
    else
      m_GridOrgans->DeselectRow(i);
    if (rec.is_focused)
      m_GridOrgans->SetGridCursor(i, 0);
  }
}

static wxString NAMES_DELIM = "\n";

void GOSettingsOrgans::DelSelectedOrgans() {
  const wxArrayInt selectedItems = m_GridOrgans->GetSelectedRows();

  if (selectedItems.size()) {
    std::unordered_set<unsigned> selectedItemSet;
    wxString organNames;

    for (int i : selectedItems) {
      selectedItemSet.insert(i);
      organNames += NAMES_DELIM
        + m_OrganSlotPtrsByGridLine[i]->p_CurrentOrgan->GetChurchName();
    }

    // find all package paths are still used by the remaining organs
    PackageSlotSet packagesUsed = GetUsedPackages(selectedItemSet);

    // find all packages that are not used anymore and will be removed
    std::unordered_set<PackageSlot *> packagesToDelete;
    wxString packageNamesToDelete;

    for (PackageSlot &pkgSlot : m_PackageSlots)
      if (
        pkgSlot.is_present
        && packagesUsed.find(&pkgSlot) == packagesUsed.end()) {
        packagesToDelete.insert(&pkgSlot);
        packageNamesToDelete += NAMES_DELIM + pkgSlot.p_CurrentPkg->GetName();
      }

    wxString addDelMessage = packageNamesToDelete.IsEmpty()
      ? EMPTY_STRING
      : wxString::Format(
        _("\n\n(the following packages will also be deleted:%s)"),
        packageNamesToDelete);

    if (
      wxMessageBox(
        wxString::Format(
          _("Do you want to delete organs%s%s?"), organNames, addDelMessage),
        _("Delete Organs"),
        wxYES_NO | wxICON_QUESTION,
        this)
      == wxYES) {
      // check if the package paths are outside the package directories

      const wxString autoPkgDir = m_config.OrganPackagePath();

      packageNamesToDelete.Clear();
      for (PackageSlot *pkgSlot : packagesToDelete) {
        const wxString pkgDir
          = wxFileName(pkgSlot->p_CurrentPkg->GetPath()).GetPath();

        if (pkgDir == autoPkgDir)
          packageNamesToDelete
            += pkgSlot->p_CurrentPkg->GetName() + NAMES_DELIM;
      }

      if (
        packageNamesToDelete.IsEmpty()
        || wxMessageBox(
             wxString::Format(
               _("The following organ packages\n"
                 "%s are in the Packages directory and will be registered "
                 "automatically\n"
                 "upon next GrandOrgue restart. Do you really want to "
                 "unregister them?"),
               packageNamesToDelete),
             _("Delete Organ Packages"),
             wxYES_NO | wxICON_QUESTION,
             this)
          == wxYES) {
        // do actual deleting
        auto slotsBegin = m_OrganSlotPtrsByGridLine.begin();

        for (int i = selectedItems.size() - 1; i >= 0; i--) {
          const int j = selectedItems[i];
          OrganSlot *pOrganSlot = m_OrganSlotPtrsByGridLine[j];

          m_OrganSlotPtrsByGridLine.erase(slotsBegin + j);
          m_GridOrgans->DeleteRows(j);
          pOrganSlot->is_present = false;
          m_OrganSlotByPath.erase(pOrganSlot->m_CurrentPath);
        }

        // delete the packages that are not existing among the original packages
        for (PackageSlot *pkgSlot : packagesToDelete) {
          pkgSlot->is_present = false;
          m_PackageSlotByPath.erase(pkgSlot->p_CurrentPkg->GetPath());
        }

        RefreshFocused(m_GridOrgans->GetGridCursorRow());
      }
    }
  }
}

void GOSettingsOrgans::OnOrganMidi(wxCommandEvent &event) {
  const int currOrganIndex = m_GridOrgans->GetGridCursorRow();

  if (currOrganIndex >= 0) {
    OrganSlot *pOrganSlot = m_OrganSlotPtrsByGridLine[currOrganIndex];
    GOMidiEventDialog dlg(
      NULL,
      this,
      wxString::Format(
        _("MIDI settings for organ %s"),
        pOrganSlot->p_CurrentOrgan->GetChurchName()),
      m_config,
      wxT("Organs"),
      &pOrganSlot->p_CurrentOrgan->GetMIDIReceiver(),
      NULL,
      NULL);

    dlg.RegisterMIDIListener(&m_midi);
    dlg.ShowModal();
    DisplayMidiCell(currOrganIndex, pOrganSlot->p_CurrentOrgan);
  }
}

void GOSettingsOrgans::ReplaceOrganPath(
  const unsigned index, const wxString &newPath) {
  OrganSlot *pOrganSlot = m_OrganSlotPtrsByGridLine[index];

  if (pOrganSlot->m_CurrentPath != newPath) {
    GOOrgan *pNewOrgan = new GOOrgan(*pOrganSlot->p_CurrentOrgan);

    if (pOrganSlot->is_packaged)
      pNewOrgan->SetArchivePath(newPath);
    else
      pNewOrgan->SetODFPath(newPath);
    DisplayPathCell(index, newPath);

    m_OrganSlotByPath.erase(pOrganSlot->m_CurrentPath);

    // delete pOrganSlot->p_CurrentOrgan if it was created during a previous
    // relocate
    if (pOrganSlot->p_CurrentOrgan != pOrganSlot->p_OrigOrgan)
      delete pOrganSlot->p_CurrentOrgan;
    pOrganSlot->p_CurrentOrgan = pNewOrgan;
    pOrganSlot->m_CurrentPath = newPath;
    m_OrganSlotByPath[newPath] = pOrganSlot;
  }
}

const wxString WX_GRID_ORGANS = wxT("GridOrgans");

void GOSettingsOrgans::ApplyAdditionalSizes(
  const GOAdditionalSizeKeeper &sizeKeeper) {
  GOAdditionalSizeKeeperProxy proxyGridOrgans(
    const_cast<GOAdditionalSizeKeeper &>(sizeKeeper), WX_GRID_ORGANS);

  m_GridOrgans->ApplyColumnSizes(proxyGridOrgans);
}

void GOSettingsOrgans::CaptureAdditionalSizes(
  GOAdditionalSizeKeeper &sizeKeeper) const {
  GOAdditionalSizeKeeperProxy proxyGridOrgans(sizeKeeper, WX_GRID_ORGANS);

  m_GridOrgans->CaptureColumnSizes(proxyGridOrgans);
}

void GOSettingsOrgans::OnOrganUp(wxCommandEvent &event) {
  VisibleOrganRecs recs(GetCurrentOrganRecs());
  const int l = recs.size();

  if (l && !recs[0].is_selected)
    for (int i = 1; i < l; i++) {
      VisibleOrganRec &rec = recs[i];

      if (rec.is_selected)
        std::swap(rec, recs[i - 1]);
    }
  ReorderOrgans(recs);
}

void GOSettingsOrgans::OnOrganDown(wxCommandEvent &event) {
  VisibleOrganRecs recs(GetCurrentOrganRecs());
  const int l = recs.size();

  if (l && !recs[l - 1].is_selected)
    for (int i = l - 2; i >= 0; i--) {
      VisibleOrganRec &rec = recs[i];

      if (rec.is_selected)
        std::swap(rec, recs[i + 1]);
    }
  ReorderOrgans(recs);
}

void GOSettingsOrgans::OnOrganTop(wxCommandEvent &event) {
  VisibleOrganRecs oldRecs(GetCurrentOrganRecs());
  VisibleOrganRecs newRecs;

  // add all selected to the new order
  std::copy_if(
    oldRecs.begin(),
    oldRecs.end(),
    std::back_inserter(newRecs),
    [](VisibleOrganRec &rec) { return rec.is_selected; });

  // add all unselected to the new order
  std::copy_if(
    oldRecs.begin(),
    oldRecs.end(),
    std::back_inserter(newRecs),
    [](VisibleOrganRec &rec) { return !rec.is_selected; });
  ReorderOrgans(newRecs);
}

void GOSettingsOrgans::OnOrganRelocate(wxCommandEvent &event) {
  const int currOrganIndex = m_GridOrgans->GetGridCursorRow();

  if (currOrganIndex >= 0) {
    OrganSlot *pOrganSlot = m_OrganSlotPtrsByGridLine[currOrganIndex];
    const bool isArchive = pOrganSlot->is_packaged;
    const wxString oldPath = pOrganSlot->m_CurrentPath;
    const wxFileName oldFileName(oldPath);
    const wxString &wildcard = isArchive
      ? GOStdFileName::getPackageDlgWildcard()
      : GOStdFileName::getOdfDlgWildcard();

    // select a new filename
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
        bool isAlreadyUsed
          = m_OrganSlotByPath.find(newPath) != m_OrganSlotByPath.end();

        if (isAlreadyUsed)
          wxMessageBox(
            _("Another registered organ already uses this path"),
            _("Relocating path"),
            wxICON_STOP | wxOK);
        else if (isArchive && pOrganSlot->p_PackageSlot) {
          m_PackageSlotByPath.erase(oldPath);

          PackageSlot *pPkgSlot = pOrganSlot->p_PackageSlot;
          GOArchiveFile *pNewPkg = new GOArchiveFile(*pPkgSlot->p_CurrentPkg);

          pNewPkg->SetPath(newPath);
          m_PackageSlotByPath[newPath] = pPkgSlot;

          // delete pOldPkg if it has been created in this dialog
          if (pPkgSlot->p_CurrentPkg != pPkgSlot->p_OrigPkg)
            delete pPkgSlot->p_CurrentPkg;
          pPkgSlot->p_CurrentPkg = pNewPkg;

          // switch other organs from the same package to the new archive
          for (unsigned l = m_OrganSlotPtrsByGridLine.size(), i = 0; i < l; i++)
            if (m_OrganSlotPtrsByGridLine[i]->m_CurrentPath == oldPath)
              ReplaceOrganPath(i, newPath);
        } else {
          ReplaceOrganPath(currOrganIndex, newPath);
          RefreshFocused(m_GridOrgans->GetGridCursorRow());
        }
      }
    }
  }
}

void GOSettingsOrgans::OnDelCache(wxCommandEvent &event) {
  for (auto i : m_GridOrgans->GetSelectedRows()) {
    OrganSlot *pOrganSlot = m_OrganSlotPtrsByGridLine[i];

    if (pOrganSlot->is_present && pOrganSlot->is_AnyCacheExisting)
      pOrganSlot->is_AnyCacheExisting = false;
  }
  m_DelCache->Disable();
}

void GOSettingsOrgans::OnDelPreset(wxCommandEvent &event) {
  for (auto i : m_GridOrgans->GetSelectedRows()) {
    OrganSlot *pOrganSlot = m_OrganSlotPtrsByGridLine[i];

    if (pOrganSlot->is_present && pOrganSlot->is_AnyPresetExisting)
      pOrganSlot->is_AnyPresetExisting = false;
  }
  m_DelPreset->Disable();
}

using StringToStringMap = std::unordered_map<wxString, wxString, wxStringHash>;

void keep_delete_rename_hash_files(
  const wxString &dirPath,
  const wxString &pattern,
  const StringToStringMap &hashesToKeep) {
  wxArrayString files;

  wxDir::GetAllFiles(dirPath, &files, pattern);

  for (const wxString &fullName : files) {
    const wxString hashFromFile = GOStdFileName::extractOrganHash(fullName);
    auto newHashIter = hashesToKeep.find(hashFromFile);

    if (newHashIter == hashesToKeep.end())
      if (wxRemoveFile(fullName))
        wxLogInfo(_("Delete file %s"), fullName);
      else
        wxLogError(_("Unable to delete file %s"), fullName);
    else if (newHashIter->second != hashFromFile) {
      wxFileName newWxName(fullName);
      const wxString newName = newHashIter->second
        + newWxName.GetFullName().Mid(hashFromFile.Length());

      newWxName.SetFullName(newName);

      wxString newFullName = newWxName.GetFullPath();

      if (wxRenameFile(fullName, newFullName, false))
        wxLogInfo(_("Renamed file %s to %s"), fullName, newFullName);
      else
        wxLogError(_("Unable to rename file %s to %s"), fullName, newFullName);
    }
  }
}

bool GOSettingsOrgans::TransferDataFromWindow() {
  StringToStringMap
    cachesToKeep; // the old hash -> the new hash; may be the same
  StringToStringMap
    presetsToKeep; // the old hash -> the new hash; may be the same
  std::unordered_set<const GOOrgan *> newOrgans;

  // populate hashesToKeep and newOrgans
  for (const OrganSlot &organSlot : m_OrganSlots)
    if (organSlot.is_present) {
      const wxString oldHash = organSlot.p_OrigOrgan->GetOrganHash();
      const wxString newHash = organSlot.p_CurrentOrgan->GetOrganHash();

      if (organSlot.is_AnyCacheExisting)
        cachesToKeep[oldHash] = newHash;
      if (organSlot.is_AnyPresetExisting)
        presetsToKeep[oldHash] = newHash;
      newOrgans.insert(organSlot.p_CurrentOrgan);
    }

  // reorder organs.Some of them may be deleted or replaced
  for (GOOrgan *&pOrgan : m_OrigOrganList) {
    if (newOrgans.find(pOrgan) == newOrgans.end())
      // actually delete the organ
      delete pOrgan;
    pOrgan = NULL; // for preventing delete on clear()
  }
  m_OrigOrganList.clear();
  for (auto pOrganSlot : m_OrganSlotPtrsByGridLine)
    m_OrigOrganList.push_back(pOrganSlot->p_CurrentOrgan);

  StringToStringMap
    indicesToKeep; // the old hash -> the new hash; may be the same

  // populate indicesToKeep
  for (const PackageSlot &pkgSlot : m_PackageSlots)
    if (pkgSlot.is_present) {
      const wxString oldHash = pkgSlot.p_OrigPkg->GetArchiveHash();
      const wxString newHash = pkgSlot.p_CurrentPkg->GetArchiveHash();

      indicesToKeep[oldHash] = newHash;
    }

  // delete or replace extra packages
  for (int i = m_OrigPackageList.size() - 1; i >= 0; i--) {
    PackageSlot &pkgSlot = m_PackageSlots[i];

    if (!pkgSlot.is_present) {
      m_PackageSlots.erase(m_PackageSlots.begin() + i);
      m_OrigPackageList.erase(i);
    } else if (pkgSlot.p_CurrentPkg != pkgSlot.p_OrigPkg) {
      m_OrigPackageList[i] = pkgSlot.p_CurrentPkg;
      delete pkgSlot.p_OrigPkg;
      pkgSlot.p_OrigPkg = pkgSlot.p_CurrentPkg;
    }
  }

  // keep/delete/rename cache and preset and index files
  keep_delete_rename_hash_files(
    m_config.OrganCachePath(),
    GOStdFileName::composeCacheFilePattern(),
    cachesToKeep);
  keep_delete_rename_hash_files(
    m_config.OrganSettingsPath(),
    GOStdFileName::composeSettingFilePattern(),
    presetsToKeep);
  keep_delete_rename_hash_files(
    m_config.OrganCachePath(),
    GOStdFileName::composeIndexFilePattern(),
    indicesToKeep);

  return true;
}
