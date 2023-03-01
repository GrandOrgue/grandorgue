/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalSetter.h"

#include <wx/string.h>
#include <wx/translation.h>

#include "control/GOButtonControl.h"
#include "control/GOLabelControl.h"
#include "model/GODivisionalCombination.h"
#include "model/GOManual.h"

#include "GOOrganController.h"
#include "GOSetter.h"

#define DIVISIONAL_BANKS 20

// ids for a single manual
// id for any manual is obtained by adding N_BUTTONS * manualN
enum {
  ID_DIVISIONAL01 = 0,
  ID_DIVISIONAL02,
  ID_DIVISIONAL03,
  ID_DIVISIONAL04,
  ID_DIVISIONAL05,
  ID_DIVISIONAL06,
  ID_DIVISIONAL07,
  ID_DIVISIONAL08,
  ID_DIVISIONAL09,
  ID_DIVISIONAL10,
  N_DIVISIONALS,
  ID_PREV_BANK = N_DIVISIONALS,
  ID_NEXT_BANK,
  N_BUTTONS,
  ID_FIRST = 0
};

const struct GOElementCreator::ButtonDefinitionEntry *GODivisionalSetter::
  GetButtonDefinitionList() {
  return m_ButtonDefinitions;
}

// fills a button definition
void fill_button_definition(
  wxString name,
  GOElementCreator::ButtonDefinitionEntry *&pb,
  unsigned &currId) {
  pb->name = name;
  pb->value = currId++;
  pb->is_public = true;
  pb->is_pushbutton = true;
  pb->is_piston = true;
  pb++;
}

// fills a button definition
static GOElementCreator::ButtonDefinitionEntry final_button_definition_entry
  = {wxT(""), -1, false, false};

wxString GODivisionalSetter::GetDivisionalButtonName(
  unsigned manualIndex, unsigned divisionalN) {
  return wxString::Format(
    wxT("Setter%03dDivisional%03d"), manualIndex, divisionalN);
}

wxString GODivisionalSetter::GetDivisionalBankLabelName(unsigned manualIndex) {
  return wxString::Format(wxT("Setter%03dDivisionalBank"), manualIndex);
}

wxString GODivisionalSetter::GetDivisionalBankPrevLabelName(
  unsigned manualIndex) {
  return wxString::Format(wxT("Setter%03dDivisionalPrevBank"), manualIndex);
}

wxString GODivisionalSetter::GetDivisionalBankNextLabelName(
  unsigned manualIndex) {
  return wxString::Format(wxT("Setter%03dDivisionalNextBank"), manualIndex);
}

GODivisionalSetter::GODivisionalSetter(GOOrganController *organController)
  : m_OrganController(organController),
    m_FirstManualIndex(m_OrganController->GetFirstManualIndex()),
    m_OdfManualCount(m_OrganController->GetODFManualCount()),
    m_NManuals(m_OdfManualCount - m_FirstManualIndex),
    // additional 1 element for the end mark
    m_ButtonDefinitions(new ButtonDefinitionEntry[N_BUTTONS * m_NManuals + 1]) {
  // fill m_ButtonDefinitions
  ButtonDefinitionEntry *pb = m_ButtonDefinitions;
  unsigned currId = ID_DIVISIONAL01 + ID_FIRST;

  // construct button definitions
  for (unsigned odfManualIndex = m_FirstManualIndex;
       odfManualIndex < m_OdfManualCount;
       odfManualIndex++) {
    for (unsigned divisionalIndex = 0; divisionalIndex < N_DIVISIONALS;
         divisionalIndex++)
      fill_button_definition(
        GetDivisionalButtonName(odfManualIndex, divisionalIndex), pb, currId);
    fill_button_definition(
      GetDivisionalBankPrevLabelName(odfManualIndex), pb, currId);
    fill_button_definition(
      GetDivisionalBankNextLabelName(odfManualIndex), pb, currId);
  }
  *pb = final_button_definition_entry;

  // create button conrols for all buttons. It calls the GetButtonDefinitionList
  // callback
  CreateButtons(organController);

  for (unsigned manualN = 0; manualN < m_NManuals; manualN++) {
    m_manualBanks.push_back(0);
    m_BankLabels.push_back(new GOLabelControl(organController));
    m_DivisionalMaps.emplace_back();
  }
}

GODivisionalSetter::~GODivisionalSetter() {
  ClearCombinations();
  m_BankLabelsByName.clear();
  delete[] m_ButtonDefinitions;
  m_OrganController->UnregisterSaveableObject(this);
}

void GODivisionalSetter::UpdateBankDisplay(unsigned manualN) {
  // 0 -> A, 19 -> T
  m_BankLabels[manualN]->SetContent(
    wxString::Format(wxT("%c"), 'A' + m_manualBanks[manualN]));
}

void GODivisionalSetter::Save(GOConfigWriter &cfg) {
  // save all defined combinations
  for (DivisionalMap &divMap : m_DivisionalMaps)
    for (auto &divEntry : divMap)
      if (divEntry.second)
        divEntry.second->Save(cfg);
}

void GODivisionalSetter::Load(GOConfigReader &cfg) {
  m_OrganController->RegisterSaveableObject(this);
  m_BankLabelsByName.clear();

  // load for all manuals
  for (unsigned manualN = 0; manualN < m_NManuals; manualN++) {
    unsigned odfManualIndex = m_FirstManualIndex + manualN;
    GOLabelControl *const pL = m_BankLabels[manualN];
    const wxString labelName = GetDivisionalBankLabelName(odfManualIndex);

    // init the bank label
    pL->Init(
      cfg,
      labelName,
      wxString::Format(
        _("divisional bank for %s"),
        m_OrganController->GetManual(odfManualIndex)->GetName()));
    m_BankLabelsByName[labelName] = pL;

    // init the divisional buttons
    for (unsigned j = 0; j < N_DIVISIONALS; j++) {
      wxString buttonName = GetDivisionalButtonName(odfManualIndex, j);
      GOButtonControl *const divisional = GetButtonControl(buttonName, false);

      assert(divisional);
      divisional->Init(cfg, buttonName, wxString::Format(wxT("%d"), j + 1));
      divisional->Load(cfg, buttonName);
      divisional->SetDisplayed(true);
    }

    // init the prev bank button
    wxString buttonPrevName = GetDivisionalBankPrevLabelName(odfManualIndex);
    GOButtonControl *const divisionalPrev
      = GetButtonControl(buttonPrevName, false);

    divisionalPrev->Init(cfg, buttonPrevName, wxT("-"));

    // init the next bank button
    wxString buttonNextName = GetDivisionalBankNextLabelName(odfManualIndex);
    GOButtonControl *const divisionalNext
      = GetButtonControl(buttonNextName, false);

    divisionalNext->Init(cfg, buttonNextName, wxT("+"));

    // display the initial bank
    UpdateBankDisplay(manualN);
  }
}

void GODivisionalSetter::ClearCombinations() {
  for (DivisionalMap &divMap : m_DivisionalMaps) {
    for (auto &divEntry : divMap) {
      GODivisionalCombination *pCmb = divEntry.second;

      delete pCmb;
      divEntry.second = nullptr;
    }
    divMap.clear();
  }
}

static const wxString WX_EMPTY_STRING = wxEmptyString;

void GODivisionalSetter::LoadCombination(GOConfigReader &cfg) {
  ClearCombinations();
  for (unsigned manualN = 0; manualN < m_NManuals; manualN++) {
    unsigned odfManualIndex = m_FirstManualIndex + manualN;
    DivisionalMap &divMap = m_DivisionalMaps[manualN];
    GOManual *pManual = m_OrganController->GetManual(odfManualIndex);
    GOCombinationDefinition &cmbTemplate = pManual->GetDivisionalTemplate();

    for (unsigned nDivisionals = N_DIVISIONALS * DIVISIONAL_BANKS,
                  divisionalIndex = 0;
         divisionalIndex < nDivisionals;
         divisionalIndex++) {
      // try to load the combination
      GODivisionalCombination *pCmb = GODivisionalCombination::LoadFrom(
        m_OrganController,
        cfg,
        cmbTemplate,
        GetDivisionalButtonName(odfManualIndex, divisionalIndex),
        WX_EMPTY_STRING, // legacyGroupName is not used yet
        odfManualIndex,
        divisionalIndex);
      // if the combination is not defined then pCmb == NULL

      if (pCmb)
        divMap[divisionalIndex] = pCmb;
    }
  }
}

void GODivisionalSetter::SwitchDivisionalTo(
  unsigned manualN, unsigned divisionalN) {
  if (manualN < m_NManuals && divisionalN < N_DIVISIONALS) {
    // absolute index of the divisional combination for the manual
    unsigned divisionalIdx
      = N_DIVISIONALS * m_manualBanks[manualN] + divisionalN;
    DivisionalMap &divMap = m_DivisionalMaps[manualN];
    // whether the combination is defined
    bool isExist = divMap.find(divisionalIdx) != divMap.end();
    GODivisionalCombination *pCmb = isExist ? divMap[divisionalIdx] : nullptr;

    if (!isExist && m_OrganController->GetSetter()->IsSetterActive()) {
      // create a new combination
      const unsigned manualIndex = m_FirstManualIndex + manualN;
      GOCombinationDefinition &divTemplate
        = m_OrganController->GetManual(manualIndex)->GetDivisionalTemplate();

      pCmb = new GODivisionalCombination(m_OrganController, divTemplate, true);
      pCmb->Init(
        GetDivisionalButtonName(manualIndex, divisionalIdx),
        manualIndex,
        divisionalIdx);
      divMap[divisionalIdx] = pCmb;
    }

    if (pCmb) {
      // the combination was existing or has just been created
      pCmb->Push();

      // reflect the ne state of the combination buttons
      for (unsigned firstButtonIdx = N_BUTTONS * manualN, k = 0;
           k < N_DIVISIONALS;
           k++) {
        GOButtonControl *divisional = m_buttons[firstButtonIdx + k];

        divisional->Display(k == divisionalN);
      }
    }
  }
}

void GODivisionalSetter::SwitchBankToPrev(unsigned manualN) {
  if (manualN < m_NManuals) {
    unsigned &currBank = m_manualBanks[manualN];

    if (currBank > 0) {
      currBank--;
      UpdateBankDisplay(manualN);
    }
  }
}

void GODivisionalSetter::SwitchBankToNext(unsigned manualN) {
  if (manualN < m_NManuals) {
    unsigned &currBank = m_manualBanks[manualN];

    if (currBank < DIVISIONAL_BANKS - 1) {
      currBank++;
      UpdateBankDisplay(manualN);
    }
  }
}

void GODivisionalSetter::ButtonStateChanged(int id) {
  // dispatch button press to the actual button

  int manualN = id / N_BUTTONS;
  int buttonId = id % N_BUTTONS;

  if (manualN >= 0 && (unsigned)manualN < m_NManuals) {
    if (buttonId >= 0 && buttonId < N_DIVISIONALS)
      SwitchDivisionalTo(manualN, buttonId);
    else if (buttonId == ID_PREV_BANK)
      SwitchBankToPrev(manualN);
    else if (buttonId == ID_NEXT_BANK)
      SwitchBankToNext(manualN);
  }
}
