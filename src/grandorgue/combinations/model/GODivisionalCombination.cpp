/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalCombination.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfigWriter.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "yaml/go-wx-yaml.h"

GODivisionalCombination::GODivisionalCombination(
  GOOrganModel &organModel, unsigned manualNumber, bool isSetter)
  : GOCombination(
    organModel, organModel.GetManual(manualNumber)->GetDivisionalTemplate()),
    r_OrganModel(organModel),
    m_odfManualNumber(manualNumber),
    m_DivisionalNumber(0),
    m_IsSetter(isSetter) {}

void GODivisionalCombination::Init(
  const wxString &group, int divisionalNumber) {
  m_group = group;
  m_DivisionalNumber = divisionalNumber;
  m_Protected = false;
}

void GODivisionalCombination::Load(
  GOConfigReader &cfg, const wxString &group, int divisionalNumber) {
  Init(group, divisionalNumber);
  m_Protected
    = cfg.ReadBoolean(ODFSetting, group, wxT("Protected"), false, false);

  if (!m_IsSetter)
    /* check ODF settings */
    LoadCombination(cfg, ODFSetting);
}

void GODivisionalCombination::LoadCombinationInt(
  GOConfigReader &cfg, GOSettingType srcType) {
  GOManual *associatedManual = r_OrganModel.GetManual(m_odfManualNumber);
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
    r_OrganModel.DivisionalsStoreIntermanualCouplers()
      || r_OrganModel.DivisionalsStoreIntramanualCouplers(),
    0);
  unsigned NumberOfTremulants = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    r_OrganModel.GetTremulantCount(),
    r_OrganModel.DivisionalsStoreTremulants(),
    0);
  unsigned NumberOfSwitches = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    r_OrganModel.GetSwitchCount(),
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

  EnsureElementStatesAllocated();
  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
    const GOCombinationDefinition::Element &e = r_ElementDefinitions[i];
    GOBool3 state = GetElementState(i);

    if (state >= BOOL3_FALSE) {
      int value = to_bool(state) ? e.index : -e.index;

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
  GOManual &manual = *r_OrganModel.GetManual(m_odfManualNumber);

  switch (e.type) {
  case GOCombinationDefinition::COMBINATION_STOP:
    yamlMap[STOPS][valueLabel] = manual.GetStop(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_COUPLER:
    yamlMap[COUPLERS][valueLabel] = manual.GetCoupler(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_TREMULANT:
    yamlMap[TREMULANTS][valueLabel]
      = r_OrganModel.GetTremulant(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_SWITCH:
    yamlMap[SWITCHES][valueLabel] = manual.GetSwitch(objectIndex)->GetName();
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

GODivisionalCombination *GODivisionalCombination::loadFrom(
  GOOrganModel &organModel,
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &readGroup,
  int manualNumber,
  int divisionalNumber) {
  GODivisionalCombination *pCmb = nullptr;
  bool isCmbOnGroup = isCmbOnFile(cfg, group);
  bool isCmbOnReadGroup = !readGroup.IsEmpty() && isCmbOnFile(cfg, readGroup);

  if (isCmbOnGroup || isCmbOnReadGroup) {
    pCmb = new GODivisionalCombination(organModel, manualNumber, false);
    pCmb->Load(cfg, isCmbOnReadGroup ? readGroup : group, divisionalNumber);
    pCmb->LoadCombination(cfg);
    if (isCmbOnReadGroup) {  // The combination was loaded from the legacy group
      pCmb->m_group = group; // It will be saved to the normal group
      if (isCmbOnGroup)      // Load the overriden combination
        pCmb->LoadCombination(cfg);
    }
  }
  return pCmb;
}
