/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalSetter.h"

#include <wx/string.h>
#include <wx/translation.h>

#include "control/GOButtonControl.h"
#include "control/GODivisionalButtonControl.h"
#include "control/GOLabelControl.h"
#include "model/GODivisionalCombination.h"
#include "model/GOManual.h"
#include "model/GOSetterState.h"
#include "yaml/go-wx-yaml.h"

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

GODivisionalSetter::GODivisionalSetter(
  GOOrganController *organController, const GOSetterState &setterState)
  : m_OrganController(organController),
    r_SetterState(setterState),
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
  CreateButtons(*organController);
  organController->RegisterCombinationButtonSet(this);
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

void GODivisionalSetter::UpdateBankDisplay(unsigned manualN, uint8_t bankN) {
  // 0 -> A, 19 -> T
  m_BankLabels[manualN]->SetContent(wxString::Format(wxT("%c"), 'A' + bankN));
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
    m_manualBanks[manualN] = 0;
    UpdateBankDisplay(manualN, 0);
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

    for (unsigned nDivisionals = N_DIVISIONALS * DIVISIONAL_BANKS,
                  divisionalIndex = 0;
         divisionalIndex < nDivisionals;
         divisionalIndex++) {
      // try to load the combination
      GODivisionalCombination *pCmb = GODivisionalCombination::loadFrom(
        *m_OrganController,
        cfg,
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

const char *const DIVISIONALS = "divisionals";
const char *const BANKED_DIVISIONALS = "banked-divisionals";
const char *const NAME = "name";
const char *const COMBINATIONS = "combinations";
const wxString WX_MANUALP03U = wxT("Manual%03u");
const wxString WX_PU = wxT("%u");
const wxString WX_P03U = wxT("%03u");
const wxString WX_PCP02U = wxT("%c%02u");

wxString manual_yaml_key(unsigned odfManualIndex) {
  return wxString::Format(WX_MANUALP03U, odfManualIndex);
}

wxString divisional_yaml_key(unsigned i) {
  return wxString::Format(WX_PU, i + 1);
}

wxString banked_divisional_yaml_key(unsigned i) {
  return wxString::Format(
    WX_PCP02U, i / N_DIVISIONALS + 'A', i % N_DIVISIONALS + 1);
}

unsigned banked_divisional_yaml_key_to_index(const wxString &key) {
  return N_DIVISIONALS * (key[0].GetValue() - 'A') + wxAtoi(key.Mid(1)) - 1;
}

void GODivisionalSetter::ToYaml(YAML::Node &yamlNode) const {
  YAML::Node divisionalsNode;
  YAML::Node bankedDivisionalsNode;

  for (unsigned manualN = 0; manualN < m_NManuals; manualN++) {
    unsigned odfManualIndex = m_FirstManualIndex + manualN;
    GOManual *const pManual = m_OrganController->GetManual(odfManualIndex);
    const wxString &manualName = pManual->GetName();
    const wxString manualLabel = manual_yaml_key(odfManualIndex);
    const DivisionalMap &divMap = m_DivisionalMaps[manualN];

    // simple divisionals
    YAML::Node simpleCmbsNode;

    for (unsigned l = pManual->GetDivisionalCount(), i = 0; i < l; i++)
      pManual->GetDivisional(i)->GetCombination().PutToYamlMap(
        simpleCmbsNode, divisional_yaml_key(i));
    put_to_map_with_name(
      divisionalsNode, manualLabel, manualName, COMBINATIONS, simpleCmbsNode);

    // banked divisionals
    YAML::Node bankedCmbsNode;

    for (auto &divEntry : divMap)
      GOCombination::putToYamlMap(
        bankedCmbsNode,
        banked_divisional_yaml_key(divEntry.first),
        divEntry.second);
    put_to_map_with_name(
      bankedDivisionalsNode,
      manualLabel,
      manualName,
      COMBINATIONS,
      bankedCmbsNode);
  }
  put_to_map_if_not_null(yamlNode, DIVISIONALS, divisionalsNode);
  put_to_map_if_not_null(yamlNode, BANKED_DIVISIONALS, bankedDivisionalsNode);
}

void GODivisionalSetter::FromYaml(const YAML::Node &yamlNode) {
  const YAML::Node divisionalsNode = yamlNode[DIVISIONALS];
  const YAML::Node bankedDivisionalsNode = yamlNode[BANKED_DIVISIONALS];

  for (unsigned manualN = 0; manualN < m_NManuals; manualN++) {
    unsigned odfManualIndex = m_FirstManualIndex + manualN;
    const wxString manualLabel = manual_yaml_key(odfManualIndex);
    GOManual *const pManual = m_OrganController->GetManual(odfManualIndex);
    DivisionalMap &divMap = m_DivisionalMaps[manualN];

    // simple divisionals
    const YAML::Node simpleCmbsNode = get_from_map_or_null(
      get_from_map_or_null(divisionalsNode, manualLabel), COMBINATIONS);

    for (unsigned l = pManual->GetDivisionalCount(), i = 0; i < l; i++)
      get_from_map_or_null(simpleCmbsNode, divisional_yaml_key(i))
        >> pManual->GetDivisional(i)->GetCombination();

    // banked divisionals
    const YAML::Node bankedCmbsNode = get_from_map_or_null(
      get_from_map_or_null(bankedDivisionalsNode, manualLabel), COMBINATIONS);

    divMap.clear();
    for (const auto &cmbEntry : bankedCmbsNode) {
      const YAML::Node &cmbNode = cmbEntry.second;

      if (cmbNode.IsMap()) {
        unsigned i
          = banked_divisional_yaml_key_to_index(cmbEntry.first.as<wxString>());
        GODivisionalCombination *pCmb = new GODivisionalCombination(
          *m_OrganController, odfManualIndex, false);

        pCmb->Init(GetDivisionalButtonName(odfManualIndex, i), i);
        cmbNode >> *pCmb;
        divMap[i] = pCmb;
      }
    }
  }
}

template <typename F>
void GODivisionalSetter::SwitchBank(unsigned manualN, const F &setNewBank) {
  if (manualN < m_NManuals) {
    uint8_t &currBank = m_manualBanks[manualN];
    uint8_t oldBank = currBank;

    setNewBank(currBank);

    if (currBank != oldBank)
      UpdateBankDisplay(manualN, currBank);
  }
}

void GODivisionalSetter::SwitchBankToPrev(unsigned manualN) {
  SwitchBank(manualN, [](uint8_t &currBank) {
    if (currBank > 0)
      currBank--;
  });
}

void GODivisionalSetter::SwitchBankToNext(unsigned manualN) {
  SwitchBank(manualN, [](uint8_t &currBank) {
    if (currBank < DIVISIONAL_BANKS - 1)
      currBank++;
  });
}

void GODivisionalSetter::SwitchDivisionalTo(
  unsigned manualN, unsigned divisionalN) {
  if (manualN < m_NManuals && divisionalN < N_DIVISIONALS) {
    uint8_t bankN = m_manualBanks[manualN];
    // absolute index of the divisional combination for the manual
    unsigned divisionalIdx = N_DIVISIONALS * bankN + divisionalN;
    DivisionalMap &divMap = m_DivisionalMaps[manualN];
    // whether the combination is defined
    bool isExist = divMap.find(divisionalIdx) != divMap.end();
    GODivisionalCombination *pCmb = isExist ? divMap[divisionalIdx] : nullptr;
    const unsigned manualIndex = m_FirstManualIndex + manualN;

    if (!isExist && r_SetterState.m_IsActive) {
      // create a new combination

      pCmb = new GODivisionalCombination(*m_OrganController, manualIndex, true);
      pCmb->Init(
        GetDivisionalButtonName(manualIndex, divisionalIdx), divisionalIdx);
      divMap[divisionalIdx] = pCmb;
    }

    if (pCmb) {
      // the combination was existing or has just been created
      for (unsigned coupledManualIndex :
           m_OrganController->GetCoupledManualsForDivisional(manualIndex)) {
        unsigned coupledManualN = coupledManualIndex - m_FirstManualIndex;
        DivisionalMap &coupledDivMap = m_DivisionalMaps[coupledManualN];

        if (coupledDivMap.find(divisionalIdx) != coupledDivMap.end()) {
          if (!r_SetterState.m_IsActive) {
            // ensure that coupledManualN has the same current bank
            SwitchBank(
              coupledManualN, [=](uint8_t &currBank) { currBank = bankN; });
          } // else PushDivisional does nothing for coupled manuals
          m_OrganController->PushDivisional(
            *coupledDivMap[divisionalIdx],
            manualIndex,
            coupledManualN,
            m_buttons[N_BUTTONS * coupledManualN + divisionalN]);
        }
      }
    }
  }
}

void GODivisionalSetter::ButtonStateChanged(int id, bool newState) {
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

void GODivisionalSetter::UpdateAllButtonsLight(
  GOButtonControl *buttonToLight, int manualIndexOnlyFor) {
  for (unsigned manualN = 0; manualN < m_NManuals; manualN++) {
    unsigned odfManualIndex = m_FirstManualIndex + manualN;

    if (
      manualIndexOnlyFor < 0
      || (unsigned)manualIndexOnlyFor == odfManualIndex) {
      // reflect the new state of the combination buttons
      for (unsigned firstButtonIdx = N_BUTTONS * manualN, k = 0;
           k < N_DIVISIONALS;
           k++) {
        GOButtonControl *pDivisional = m_buttons[firstButtonIdx + k];

        pDivisional->Display(pDivisional == buttonToLight);
      }
    }
  }
}
