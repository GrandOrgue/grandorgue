/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganSettingsEnclosuresTab.h"

#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>

#include "model/GOEnclosure.h"
#include "model/GOOrganModel.h"
#include "model/GOWindchest.h"

#include "GOEvent.h"

static const wxSize EDIT_SIZE = wxSize(60, -1);
static const wxString WX_TAB_CODE = wxT("Enclosures");
static const wxString WX_TAB_TITLE = _("Enclosures");

enum {
  ID_EVENT_TREE = 200,
  ID_EVENT_MIN_AMP_LEVEL,
};

BEGIN_EVENT_TABLE(GOOrganSettingsEnclosuresTab, wxPanel)
EVT_TREE_SEL_CHANGING(
  ID_EVENT_TREE, GOOrganSettingsEnclosuresTab::OnTreeChanging)
EVT_TREE_SEL_CHANGED(ID_EVENT_TREE, GOOrganSettingsEnclosuresTab::OnTreeChanged)
EVT_TEXT(
  ID_EVENT_MIN_AMP_LEVEL, GOOrganSettingsEnclosuresTab::OnMinAmpLevelChanged)
END_EVENT_TABLE()

class GOOrganSettingsEnclosuresTab::ItemData : public wxTreeItemData {
public:
  enum ItemType {
    ORGAN,
    WINDCHEST,
    ENCLOSURE,
  } m_type;

  union {
    GOOrganModel *p_organ;
    GOWindchest *p_windchest;
    GOEnclosure *p_enclosure;
  };

  ItemData(GOOrganModel &organModel) : m_type(ORGAN), p_organ(&organModel) {}
  ItemData(GOWindchest &windchest)
    : m_type(WINDCHEST), p_windchest(&windchest) {}
  ItemData(GOEnclosure &enclosure)
    : m_type(ENCLOSURE), p_enclosure(&enclosure) {}
};

GOOrganSettingsEnclosuresTab::GOOrganSettingsEnclosuresTab(
  GOOrganModel &organModel, GOOrganSettingsDialogBase *pDlg)
  : GOOrganSettingsTab(pDlg, WX_TAB_CODE, WX_TAB_TITLE),
    r_OrganModel(organModel) {
  wxGridBagSizer *const mainSizer = new wxGridBagSizer(5, 5);

  m_tree = new wxTreeCtrl(
    this,
    ID_EVENT_TREE,
    wxDefaultPosition,
    wxDefaultSize,
    wxTR_MULTIPLE | wxTR_HIDE_ROOT);
  mainSizer->Add(
    m_tree, wxGBPosition(0, 0), wxGBSpan(4, 1), wxEXPAND | wxALL, 5);

  m_IsOdfDefined = new wxCheckBox(
    this, wxID_ANY, _("This enclosure is ODF defined and may not be altered"));
  m_IsOdfDefined->Disable();

  mainSizer->Add(
    m_IsOdfDefined,
    wxGBPosition(0, 1),
    wxGBSpan(1, 3),
    wxALIGN_LEFT | wxEXPAND | wxALL,
    5);
  mainSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Affected windchests:")),
    wxGBPosition(1, 1),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxTOP | wxLEFT | wxBOTTOM,
    5);
  m_WindchestList = new wxListBox(
    this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
  m_WindchestList->Disable();
  mainSizer->Add(
    m_WindchestList,
    wxGBPosition(1, 2),
    wxGBSpan(2, 2),
    wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM,
    5);
  mainSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Minimum amplitude level:")),
    wxGBPosition(3, 1),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxBOTTOM,
    5);
  m_MinAmpLevelEdit = new wxTextCtrl(
    this, ID_EVENT_MIN_AMP_LEVEL, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  mainSizer->Add(
    m_MinAmpLevelEdit,
    wxGBPosition(3, 2),
    wxDefaultSpan,
    wxEXPAND | wxRIGHT | wxBOTTOM,
    5);

  mainSizer->AddGrowableCol(0, 1);
  mainSizer->AddGrowableCol(3, 1);
  mainSizer->AddGrowableRow(2, 1);
  SetSizerAndFit(mainSizer);
}

bool GOOrganSettingsEnclosuresTab::TransferDataToWindow() {
  auto rootItem = m_tree->AddRoot(
    r_OrganModel.GetRootPipeConfigNode().GetName(),
    -1,
    -1,
    new ItemData(r_OrganModel));

  // fill m_WindchestsByEnclosures
  for (GOWindchest *pW : r_OrganModel.GetWindchests())
    for (GOEnclosure *pE : pW->GetEnclosures())
      m_WindchestsByEnclosures[pE].push_back(pW->GetName());
  for (GOEnclosure *pE : r_OrganModel.GetEnclosures())
    m_tree->AppendItem(rootItem, pE->GetName(), -1, -1, new ItemData(*pE));
  return true;
}

void GOOrganSettingsEnclosuresTab::OnTreeChanging(wxTreeEvent &e) {
  if (CheckForUnapplied())
    e.Veto();
}

void GOOrganSettingsEnclosuresTab::LoadValues() {
  wxArrayTreeItemIds entries;
  GOEnclosure *pSelectedEnclosure = nullptr;

  m_tree->GetSelections(entries);

  const unsigned nSelected = entries.size();

  // set pSelectedEnclosure if only one enclosure is selected
  if (nSelected == 1) {
    ItemData *pData = (ItemData *)m_tree->GetItemData(entries[0]);

    if (pData->m_type == ItemData::ENCLOSURE)
      pSelectedEnclosure = pData->p_enclosure;
  }

  // display data of the selected enclosure
  m_WindchestList->Clear();
  if (pSelectedEnclosure) {
    if (auto iWindchests = m_WindchestsByEnclosures.find(pSelectedEnclosure);
        iWindchests != m_WindchestsByEnclosures.end())
      m_WindchestList->Append(iWindchests->second);
    m_MinAmpLevelEdit->ChangeValue(
      wxString::Format("%u", pSelectedEnclosure->GetAmpMinimumLevel()));
    m_MinAmpLevelEdit->DiscardEdits();
  } else {
    m_MinAmpLevelEdit->Clear();
  }

  // check that all selected items are internal enclosures
  bool areInternalEnclosuresSelected = false;
  bool areOnlyEnclosuresSelected = false;

  for (auto id : entries) {
    ItemData *pData = (ItemData *)m_tree->GetItemData(id);
    bool isEnclosure = pData->m_type == ItemData::ENCLOSURE;
    bool isOdfDefined = isEnclosure && pData->p_enclosure->IsOdfDefined();

    if (isEnclosure) {
      areOnlyEnclosuresSelected = true;
      if (!isOdfDefined)
        areInternalEnclosuresSelected = true;
    }

    if (!isEnclosure || isOdfDefined) {
      // now we do not allow to change min value of odf-defined enclosures
      areInternalEnclosuresSelected = false;
      if (!isEnclosure)
        areOnlyEnclosuresSelected = false;
      break;
    }
  }
  m_IsOdfDefined->SetValue(
    areOnlyEnclosuresSelected && !areInternalEnclosuresSelected);
  m_MinAmpLevelEdit->Enable(areInternalEnclosuresSelected);
  m_IsDefaultEnabled = areInternalEnclosuresSelected;
  NotifyModified(false);
}

void GOOrganSettingsEnclosuresTab::DoForAllEnclosures(
  const std::function<void(GOEnclosure &enclosure)> &f) {
  wxArrayTreeItemIds entries;

  m_tree->GetSelections(entries);
  for (auto id : entries) {
    ItemData *pData = (ItemData *)m_tree->GetItemData(id);

    if (pData->m_type == ItemData::ENCLOSURE)
      f(*(pData->p_enclosure));
  }
}

void GOOrganSettingsEnclosuresTab::ResetToDefault() {
  DoForAllEnclosures([](GOEnclosure &enclosure) {
    enclosure.SetAmpMinimumLevel(enclosure.GetDefaultAmpMinimumLevel());
  });
  LoadValues();
}

void GOOrganSettingsEnclosuresTab::ApplyChanges() {
  if (m_MinAmpLevelEdit->IsModified()) {
    long minAmpVal;

    if (
      m_MinAmpLevelEdit->GetValue().ToLong(&minAmpVal) && minAmpVal >= 0
      && minAmpVal <= 100)
      DoForAllEnclosures([minAmpVal](GOEnclosure &enclosure) {
        enclosure.SetAmpMinimumLevel(minAmpVal);
      });
    else
      GOMessageBox(
        _("Minimal amplitude level is invalid"),
        _("Error"),
        wxOK | wxICON_ERROR,
        this);
    NotifyModified(false);
  }
}
