/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganSettingsEnclosuresTab.h"

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
    wxTR_HAS_BUTTONS | wxTR_MULTIPLE);
  mainSizer->Add(
    m_tree, wxGBPosition(0, 0), wxGBSpan(3, 1), wxEXPAND | wxALL, 5);

  mainSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Affected windchests:")),
    wxGBPosition(0, 1),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxTOP | wxLEFT | wxBOTTOM,
    5);
  m_WindchestList = new wxListBox(
    this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
  mainSizer->Add(
    m_WindchestList,
    wxGBPosition(0, 2),
    wxGBSpan(2, 2),
    wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM,
    5);
  mainSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Minimal amplitude level:")),
    wxGBPosition(2, 1),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxBOTTOM,
    5);
  m_MinAmpLevelEdit = new wxTextCtrl(
    this, ID_EVENT_MIN_AMP_LEVEL, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  mainSizer->Add(
    m_MinAmpLevelEdit,
    wxGBPosition(2, 2),
    wxDefaultSpan,
    wxEXPAND | wxRIGHT | wxBOTTOM,
    5);

  mainSizer->AddGrowableCol(0, 1);
  mainSizer->AddGrowableCol(3, 1);
  mainSizer->AddGrowableRow(1, 1);
  SetSizerAndFit(mainSizer);
}

bool GOOrganSettingsEnclosuresTab::TransferDataToWindow() {
  auto rootItem = m_tree->AddRoot(
    r_OrganModel.GetRootPipeConfigNode().GetName(),
    -1,
    -1,
    new ItemData(r_OrganModel));

  for (GOWindchest *pW : r_OrganModel.GetWindchests()) {
    auto windchestItem
      = m_tree->AppendItem(rootItem, pW->GetName(), -1, -1, new ItemData(*pW));

    for (GOEnclosure *pE : pW->GetEnclosures()) {
      m_tree->AppendItem(
        windchestItem, pE->GetName(), -1, -1, new ItemData(*pE));
      m_WindchestsByEnclosures[pE].push_back(pW->GetName());
    }
  }
  m_tree->Expand(rootItem);
  m_tree->SelectItem(rootItem, true);
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

  for (auto id : entries) {
    ItemData *pData = (ItemData *)m_tree->GetItemData(id);

    if (
      pData->m_type != ItemData::ENCLOSURE
      || pData->p_enclosure->IsOdfDefined()) {
      // now we do not allow to change min value of odf-defined enclosures
      areInternalEnclosuresSelected = false;
      break;
    }
    areInternalEnclosuresSelected = true;
  }
  m_MinAmpLevelEdit->Enable(areInternalEnclosuresSelected);
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
    unsigned minAmpVal;

    if (
      m_MinAmpLevelEdit->GetValue().ToUInt(&minAmpVal) && minAmpVal >= 0
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
