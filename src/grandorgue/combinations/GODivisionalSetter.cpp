/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalSetter.h"

#include <wx/string.h>
#include <wx/translation.h>

#include "control/GOButtonControl.h"
#include "control/GOLabelControl.h"
#include "model/GODivisionalCombination.h"

#include "GODefinitionFile.h"
#include "GOManual.h"
#include "GOSetter.h"

#define DIVISIONAL_BANKS 20

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

void fill_button_definition(
  wxString name,
  GOElementCreator::ButtonDefinitionEntry *&pb,
  unsigned &currId) {
  pb->name = name;
  pb->value = currId++;
  pb->is_public = true;
  pb->is_pushbutton = true;
  pb++;
}

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

GODivisionalSetter::GODivisionalSetter(GODefinitionFile *organfile)
  : m_organfile(organfile),
    m_FirstManualIndex(m_organfile->GetFirstManualIndex()),
    m_OdfManualCount(m_organfile->GetODFManualCount()),
    m_NManuals(m_OdfManualCount - m_FirstManualIndex),
    m_ButtonDefinitions(new ButtonDefinitionEntry[N_BUTTONS * m_NManuals + 1]) {
  // fill m_ButtonDefinitions
  ButtonDefinitionEntry *pb = m_ButtonDefinitions;
  unsigned currId = ID_DIVISIONAL01 + ID_FIRST;

  for (unsigned manualIndex = m_FirstManualIndex; manualIndex < m_NManuals;
       manualIndex++) {
    for (unsigned divisionalIndex = 0; divisionalIndex < N_DIVISIONALS;
         divisionalIndex++)
      fill_button_definition(
        GetDivisionalButtonName(manualIndex, divisionalIndex), pb, currId);
    fill_button_definition(
      GetDivisionalBankPrevLabelName(manualIndex), pb, currId);
    fill_button_definition(
      GetDivisionalBankNextLabelName(manualIndex), pb, currId);
  }
  *pb = final_button_definition_entry;

  CreateButtons(organfile);

  for (unsigned manualIndex = m_FirstManualIndex; manualIndex < m_NManuals;
       manualIndex++) {
    m_manualBanks.push_back(0);
    m_BankLabels.push_back(new GOLabelControl(organfile));
    m_DivisionalMaps.emplace_back();
  }
}

GODivisionalSetter::~GODivisionalSetter() {
  ClearCombinations();
  m_BankLabelsByName.clear();
  delete[] m_ButtonDefinitions;
  m_organfile->UnregisterSaveableObject(this);
}

void GODivisionalSetter::UpdateBankDisplay(unsigned manualN) {
  m_BankLabels[manualN]->SetContent(
    wxString::Format(wxT("%c"), m_manualBanks[manualN] + wxT('A')));
}

void GODivisionalSetter::Save(GOConfigWriter &cfg) {
  for (DivisionalMap &divMap : m_DivisionalMaps)
    for (auto &divEntry : divMap)
      if (divEntry.second)
        divEntry.second->Save(cfg);
}

void GODivisionalSetter::Load(GOConfigReader &cfg) {
  m_organfile->RegisterSaveableObject(this);
  m_BankLabelsByName.clear();
  for (unsigned manualN = 0; manualN < m_NManuals; manualN++) {
    unsigned manualIndex = m_FirstManualIndex + manualN;
    GOLabelControl *const pL = m_BankLabels[manualIndex - m_FirstManualIndex];
    const wxString labelName = GetDivisionalBankLabelName(manualIndex);
    GOManual *pManual = m_organfile->GetManual(manualIndex);
    unsigned nLegacyDivisionals = pManual->GetDeclaredDivisionalCount();

    pL->Init(
      cfg,
      labelName,
      wxString::Format(
        _("divisional bank for %s"),
        m_organfile->GetManual(manualIndex)->GetName()));
    m_BankLabelsByName[labelName] = pL;

    for (unsigned j = 0; j < 10; j++) {
      wxString buttonName = GetDivisionalButtonName(manualIndex, j);
      GOButtonControl *const divisional = GetButtonControl(buttonName, false);

      if (j < nLegacyDivisionals) {
        const wxString legacyGroup = pManual->GetLegacyDivisionalGroup(cfg, j);

        divisional->Load(cfg, legacyGroup);
        divisional->SetGroup(buttonName);
      } else
        divisional->Init(cfg, buttonName, wxString::Format(wxT("%d"), j + 1));
      divisional->Load(cfg, buttonName);
      divisional->SetDisplayed(true);
    }

    wxString buttonPrevName = GetDivisionalBankPrevLabelName(manualIndex);
    GOButtonControl *const divisionalPrev
      = GetButtonControl(buttonPrevName, false);

    divisionalPrev->Init(cfg, buttonPrevName, wxT("-"));

    wxString buttonNextName = GetDivisionalBankNextLabelName(manualIndex);
    GOButtonControl *const divisionalNext
      = GetButtonControl(buttonNextName, false);

    divisionalNext->Init(cfg, buttonNextName, wxT("+"));

    UpdateBankDisplay(manualIndex);
  }
  // LoadCombination(cfg);
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
    unsigned manualIndex = m_FirstManualIndex + manualN;
    DivisionalMap &divMap = m_DivisionalMaps[manualN];
    GOManual *pManual = m_organfile->GetManual(manualIndex);
    unsigned nLegacyDivisionals = pManual->GetDeclaredDivisionalCount();
    GOCombinationDefinition &cmbTemplate = pManual->GetDivisionalTemplate();

    for (unsigned nDivisionals = N_DIVISIONALS * DIVISIONAL_BANKS,
                  divisionalIndex = 0;
         divisionalIndex < nDivisionals;
         divisionalIndex++) {
      wxString legacyGroupName = divisionalIndex < nLegacyDivisionals
        ? pManual->GetLegacyDivisionalGroup(cfg, divisionalIndex)
        : WX_EMPTY_STRING;

      GODivisionalCombination *pCmb = GODivisionalCombination::LoadFrom(
        m_organfile,
        cfg,
        cmbTemplate,
        GetDivisionalButtonName(manualIndex, divisionalIndex),
        legacyGroupName,
        manualIndex,
        divisionalIndex);
      if (pCmb)
        divMap[divisionalIndex] = pCmb;
    }
  }
}

void GODivisionalSetter::SwitchDivisionalTo(
  unsigned manualN, unsigned divisionalN) {
  if (manualN < m_NManuals && divisionalN < N_DIVISIONALS) {
    unsigned divisionalIdx
      = m_manualBanks[manualN] * N_DIVISIONALS + divisionalN;
    DivisionalMap &divMap = m_DivisionalMaps[manualN];
    bool isExist = divMap.find(divisionalIdx) != divMap.end();
    GODivisionalCombination *pCmb = isExist ? divMap[divisionalIdx] : nullptr;

    if (!isExist && m_organfile->GetSetter()->IsSetterActive()) {
      const unsigned manualIndex = m_FirstManualIndex + manualN;
      GOCombinationDefinition &divTemplate
        = m_organfile->GetManual(manualIndex)->GetDivisionalTemplate();

      pCmb = new GODivisionalCombination(m_organfile, divTemplate, true);
      pCmb->Init(
        GetDivisionalButtonName(manualIndex, divisionalIdx),
        manualIndex,
        divisionalIdx);
      divMap[divisionalIdx] = pCmb;
    }

    if (pCmb) {
      pCmb->Push();
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
