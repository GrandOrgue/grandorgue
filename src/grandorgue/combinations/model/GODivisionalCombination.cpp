/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalCombination.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "combinations/control/GODivisionalButtonControl.h"

#include "combinations/GOSetter.h"
#include "config/GOConfigWriter.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "yaml/go-wx-yaml.h"

#include "GOOrganController.h"

GODivisionalCombination::GODivisionalCombination(
  GOOrganController *organController,
  GOCombinationDefinition &divisional_template,
  bool is_setter)
  : GOCombination(divisional_template, organController),
    m_OrganController(organController),
    m_odfManualNumber(1),
    m_DivisionalNumber(0),
    m_IsSetter(is_setter) {}

void GODivisionalCombination::Init(
  const wxString &group, int manualNumber, int divisionalNumber) {
  m_group = group;
  m_odfManualNumber = manualNumber;
  m_DivisionalNumber = divisionalNumber;
  m_Protected = false;
}

void GODivisionalCombination::Load(
  GOConfigReader &cfg, wxString group, int manualNumber, int divisionalNumber) {
  Init(group, manualNumber, divisionalNumber);
  m_Protected
    = cfg.ReadBoolean(ODFSetting, group, wxT("Protected"), false, false);

  if (!m_IsSetter)
    /* check ODF settings */
    LoadCombination(cfg, ODFSetting);
}

void GODivisionalCombination::LoadCombinationInt(
  GOConfigReader &cfg, GOSettingType srcType) {
  GOManual *associatedManual = m_OrganController->GetManual(m_odfManualNumber);
  wxString buffer;
  unsigned NumberOfStops
    = ReadNumberOfStops(cfg, srcType, associatedManual->GetStopCount());
  unsigned NumberOfCouplers = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfCouplers"),
    0,
    srcType == CMBSetting ? associatedManual->GetCouplerCount()
                          : associatedManual->GetODFCouplerCount(),
    m_OrganController->DivisionalsStoreIntermanualCouplers()
      || m_OrganController->DivisionalsStoreIntramanualCouplers(),
    0);
  unsigned NumberOfTremulants = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    m_OrganController->GetTremulantCount(),
    m_OrganController->DivisionalsStoreTremulants(),
    0);
  unsigned NumberOfSwitches = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    m_OrganController->GetSwitchCount(),
    false,
    0);

  unsigned cnt = associatedManual->GetStopCount();

  for (unsigned i = 0; i < NumberOfStops; i++) {
    buffer.Printf(wxT("Stop%03d"), i + 1);
    SetLoadedState(
      m_odfManualNumber,
      GOCombinationDefinition::COMBINATION_STOP,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  cnt = srcType == CMBSetting ? associatedManual->GetCouplerCount()
                              : associatedManual->GetODFCouplerCount();
  for (unsigned i = 0; i < NumberOfCouplers; i++) {
    buffer.Printf(wxT("Coupler%03d"), i + 1);
    SetLoadedState(
      m_odfManualNumber,
      GOCombinationDefinition::COMBINATION_COUPLER,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  cnt = associatedManual->GetTremulantCount();
  for (unsigned i = 0; i < NumberOfTremulants; i++) {
    buffer.Printf(wxT("Tremulant%03d"), i + 1);
    SetLoadedState(
      m_odfManualNumber,
      GOCombinationDefinition::COMBINATION_TREMULANT,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  cnt = associatedManual->GetSwitchCount();
  for (unsigned i = 0; i < NumberOfSwitches; i++) {
    buffer.Printf(wxT("Switch%03d"), i + 1);
    SetLoadedState(
      m_odfManualNumber,
      GOCombinationDefinition::COMBINATION_SWITCH,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }
}

void GODivisionalCombination::SaveInt(GOConfigWriter &cfg) {
  wxString buffer;
  unsigned stop_count = 0;
  unsigned coupler_count = 0;
  unsigned tremulant_count = 0;
  unsigned switch_count = 0;

  UpdateState();
  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
    const GOCombinationDefinition::Element &e = r_ElementDefinitions[i];
    int state = GetState(i);

    if (state >= 0) {
      int value = state == 1 ? e.index : -e.index;
      switch (e.type) {
      case GOCombinationDefinition::COMBINATION_STOP:
        stop_count++;
        buffer.Printf(wxT("Stop%03d"), stop_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_COUPLER:
        coupler_count++;
        buffer.Printf(wxT("Coupler%03d"), coupler_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_TREMULANT:
        tremulant_count++;
        buffer.Printf(wxT("Tremulant%03d"), tremulant_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_SWITCH:
        switch_count++;
        buffer.Printf(wxT("Switch%03d"), switch_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
        break;
      }
    }
  }

  WriteNumberOfStops(cfg, stop_count);
  cfg.WriteInteger(m_group, wxT("NumberOfCouplers"), coupler_count);
  cfg.WriteInteger(m_group, wxT("NumberOfTremulants"), tremulant_count);
  cfg.WriteInteger(m_group, wxT("NumberOfSwitches"), switch_count);
}

const wxString WX_P03D = "%03d";
const char *const STOPS = "stops";
const char *const COUPLERS = "couplers";
const char *const TREMULANTS = "tremulants";
const char *const SWITCHES = "switches";

void GODivisionalCombination::PutElementToYamlMap(
  const GOCombinationDefinition::Element &e,
  const wxString &valueLabel,
  const unsigned objectIndex,
  YAML::Node &yamlMap) const {
  GOManual &manual = *m_OrganController->GetManual(m_odfManualNumber);

  switch (e.type) {
  case GOCombinationDefinition::COMBINATION_STOP:
    yamlMap[STOPS][valueLabel] = manual.GetStop(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_COUPLER:
    yamlMap[COUPLERS][valueLabel] = manual.GetCoupler(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_TREMULANT:
    yamlMap[TREMULANTS][valueLabel]
      = m_OrganController->GetTremulant(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_SWITCH:
    yamlMap[SWITCHES][valueLabel]
      = m_OrganController->GetSwitch(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
    break;
  }
}

void GODivisionalCombination::FromYamlMap(const YAML::Node &yamlMap) {
  // stops
  SetStatesFromYaml(
    yamlMap[STOPS],
    m_odfManualNumber,
    GOCombinationDefinition::COMBINATION_STOP);

  // couplers
  SetStatesFromYaml(
    yamlMap[COUPLERS],
    m_odfManualNumber,
    GOCombinationDefinition::COMBINATION_COUPLER);

  // tremulants
  SetStatesFromYaml(
    yamlMap[TREMULANTS],
    m_odfManualNumber,
    GOCombinationDefinition::COMBINATION_TREMULANT);

  // switches
  SetStatesFromYaml(
    yamlMap[SWITCHES],
    m_odfManualNumber,
    GOCombinationDefinition::COMBINATION_SWITCH);
}

bool GODivisionalCombination::Push(ExtraElementsSet const *extraSet) {
  bool changed = PushLocal(extraSet);

  /* only use divisional couples, if not in setter mode */
  if (!m_OrganController->GetSetter()->IsSetterActive()) {
    for (unsigned k = 0; k < m_OrganController->GetDivisionalCouplerCount();
         k++) {
      GODivisionalCoupler *coupler = m_OrganController->GetDivisionalCoupler(k);
      if (!coupler->IsEngaged())
        continue;

      for (unsigned i = 0; i < coupler->GetNumberOfManuals(); i++) {
        if (coupler->GetManual(i) != m_odfManualNumber)
          continue;

        for (unsigned int j = i + 1; j < coupler->GetNumberOfManuals(); j++)
          m_OrganController->GetManual(coupler->GetManual(j))
            ->GetDivisional(m_DivisionalNumber)
            ->Push();

        if (coupler->IsBidirectional()) {
          for (unsigned j = 0; j < coupler->GetNumberOfManuals(); j++) {
            if (coupler->GetManual(j) == m_odfManualNumber)
              break;
            m_OrganController->GetManual(coupler->GetManual(j))
              ->GetDivisional(m_DivisionalNumber)
              ->Push();
          }
        }
        break;
      }
    }
  }
  return changed;
}

wxString GODivisionalCombination::GetMidiType() { return _("Divisional"); }

GODivisionalCombination *GODivisionalCombination::LoadFrom(
  GOOrganController *organController,
  GOConfigReader &cfg,
  GOCombinationDefinition &divisionalTemplate,
  const wxString &group,
  const wxString &readGroup,
  int manualNumber,
  int divisionalNumber) {
  GODivisionalCombination *pCmb = nullptr;
  bool isCmbOnGroup = isCmbOnFile(cfg, group);
  bool isCmbOnReadGroup = !readGroup.IsEmpty() && isCmbOnFile(cfg, readGroup);

  if (isCmbOnGroup || isCmbOnReadGroup) {
    pCmb
      = new GODivisionalCombination(organController, divisionalTemplate, false);
    pCmb->Load(
      cfg,
      isCmbOnReadGroup ? readGroup : group,
      manualNumber,
      divisionalNumber);
    pCmb->LoadCombination(cfg);
    if (isCmbOnReadGroup) {  // The combination was loaded from the legacy group
      pCmb->m_group = group; // It will be saved to the normal group
      if (isCmbOnGroup)      // Load the overriden combination
        pCmb->LoadCombination(cfg);
    }
  }
  return pCmb;
}
