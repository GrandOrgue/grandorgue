/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGeneralCombination.h"

#include <wx/intl.h>
#include <wx/log.h>
#include <yaml-cpp/yaml.h>

#include "config/GOConfigWriter.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "yaml/go-wx-yaml.h"

GOGeneralCombination::GOGeneralCombination(
  GOOrganModel &organModel, bool isSetter)
  : GOCombination(organModel, organModel.GetGeneralTemplate()),
    m_IsSetter(isSetter) {}

void GOGeneralCombination::Load(GOConfigReader &cfg, const wxString &group) {
  r_OrganModel.RegisterSaveableObject(this);
  m_group = group;

  m_Protected
    = cfg.ReadBoolean(ODFSetting, group, wxT("Protected"), false, false);

  /* check ODF settings */
  if (!m_IsSetter)
    LoadCombination(cfg, ODFSetting);
}

const wxString WX_SWITCH_MANUAL_03D = wxT("SwitchManual%03d");

void GOGeneralCombination::LoadCombinationInt(
  GOConfigReader &cfg, GOSettingType srcType) {
  wxString buffer;
  unsigned NumberOfStops
    = ReadNumberOfStops(cfg, srcType, r_OrganModel.GetStopCount());
  unsigned NumberOfCouplers = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfCouplers"),
    0,
    srcType == CMBSetting ? r_OrganModel.GetCouplerCount()
                          : r_OrganModel.GetODFCouplerCount());
  unsigned NumberOfTremulants = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    r_OrganModel.GetTremulantCount());
  unsigned NumberOfSwitches = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    r_OrganModel.GetSwitchCount(),
    false,
    0);
  unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfDivisionalCouplers"),
    0,
    r_OrganModel.GetDivisionalCouplerCount(),
    r_OrganModel.GeneralsStoreDivisionalCouplers());
  unsigned maxManualN = r_OrganModel.GetManualAndPedalCount();

  for (unsigned i = 0; i < NumberOfStops; i++) {
    unsigned m = cfg.ReadInteger(
      srcType,
      m_group,
      wxString::Format(wxT("StopManual%03d"), i + 1),
      r_OrganModel.GetFirstManualIndex(),
      maxManualN);

    buffer.Printf(wxT("StopNumber%03d"), i + 1);
    SetLoadedState(
      m,
      GOCombinationDefinition::COMBINATION_STOP,
      cfg.ReadInteger(srcType, m_group, buffer, -999, 999),
      buffer);
  }

  for (unsigned i = 0; i < NumberOfCouplers; i++) {
    unsigned m = cfg.ReadInteger(
      srcType,
      m_group,
      wxString::Format(wxT("CouplerManual%03d"), i + 1),
      r_OrganModel.GetFirstManualIndex(),
      r_OrganModel.GetManualAndPedalCount());
    unsigned cnt = srcType == CMBSetting
      ? r_OrganModel.GetManual(m)->GetCouplerCount()
      : r_OrganModel.GetManual(m)->GetODFCouplerCount();

    buffer.Printf(wxT("CouplerNumber%03d"), i + 1);
    SetLoadedState(
      m,
      GOCombinationDefinition::COMBINATION_COUPLER,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  unsigned cnt = r_OrganModel.GetTremulantCount();

  for (unsigned i = 0; i < NumberOfTremulants; i++) {
    buffer.Printf(wxT("TremulantNumber%03d"), i + 1);
    SetLoadedState(
      -1,
      GOCombinationDefinition::COMBINATION_TREMULANT,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  cnt = r_OrganModel.GetSwitchCount();
  for (unsigned i = 0; i < NumberOfSwitches; i++) {
    int switchManualN = cfg.ReadInteger(
      srcType,
      m_group,
      wxString::Format(WX_SWITCH_MANUAL_03D, i + 1),
      -1,
      maxManualN,
      false,
      -1);

    buffer.Printf(wxT("SwitchNumber%03d"), i + 1);

    int switchNumber = cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt);

    if (switchManualN < 0 && switchNumber != 0) {
      // the switch has been saved as global. Try to convert it's number to a
      // manual switch
      GOSwitch *pSwitch = r_OrganModel.GetSwitch(abs(switchNumber) - 1);

      switchManualN = pSwitch->GetAssociatedManualN();
      if (switchManualN >= 0) // may be converted
        switchNumber = (pSwitch->GetIndexInManual() + 1)
          // copy the sign
          * ((switchNumber > 0) - (switchNumber < 0));
    }
    SetLoadedState(
      switchManualN,
      GOCombinationDefinition::COMBINATION_SWITCH,
      switchNumber,
      buffer);
  }

  cnt = r_OrganModel.GetDivisionalCouplerCount();
  for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++) {
    buffer.Printf(wxT("DivisionalCouplerNumber%03d"), i + 1);
    SetLoadedState(
      -1,
      GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }
}

void GOGeneralCombination::SaveInt(GOConfigWriter &cfg) {
  EnsureElementStatesAllocated();

  wxString buffer;
  unsigned stop_count = 0;
  unsigned coupler_count = 0;
  unsigned tremulant_count = 0;
  unsigned switch_count = 0;
  unsigned divisional_coupler_count = 0;

  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
    const GOCombinationDefinition::Element &e = r_ElementDefinitions[i];
    GOBool3 state = GetElementState(i);

    if (state >= BOOL3_FALSE) {
      int value = to_bool(state) ? e.index : -e.index;

      switch (e.type) {
      case GOCombinationDefinition::COMBINATION_STOP:
        stop_count++;
        buffer.Printf(wxT("StopManual%03d"), stop_count);
        cfg.WriteInteger(m_group, buffer, e.manual);
        buffer.Printf(wxT("StopNumber%03d"), stop_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_COUPLER:
        coupler_count++;
        buffer.Printf(wxT("CouplerManual%03d"), coupler_count);
        cfg.WriteInteger(m_group, buffer, e.manual);
        buffer.Printf(wxT("CouplerNumber%03d"), coupler_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_TREMULANT:
        tremulant_count++;
        buffer.Printf(wxT("TremulantNumber%03d"), tremulant_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_SWITCH:
        switch_count++;
        cfg.WriteInteger(
          m_group,
          wxString::Format(WX_SWITCH_MANUAL_03D, switch_count),
          e.manual);
        cfg.WriteInteger(
          m_group,
          wxString::Format(wxT("SwitchNumber%03d"), switch_count),
          value);
        break;

      case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
        divisional_coupler_count++;
        buffer.Printf(
          wxT("DivisionalCouplerNumber%03d"), divisional_coupler_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;
      }
    }
  }

  WriteNumberOfStops(cfg, stop_count);
  cfg.WriteInteger(m_group, wxT("NumberOfCouplers"), coupler_count);
  cfg.WriteInteger(m_group, wxT("NumberOfTremulants"), tremulant_count);
  cfg.WriteInteger(m_group, wxT("NumberOfSwitches"), switch_count);
  cfg.WriteInteger(
    m_group, wxT("NumberOfDivisionalCouplers"), divisional_coupler_count);
}

const char *const MANUALS = "manuals";
const char *const STOPS = "stops";
const char *const COUPLERS = "couplers";
const char *const TREMULANTS = "tremulants";
const char *const SWITCHES = "switches";
const char *const DIVISIONAL_COUPLERS = "divisional-couplers";
const char *const NAME = "name";

const wxString WX_P03D = wxT("%03d");

void GOGeneralCombination::PutElementToYamlMap(
  const GOCombinationDefinition::Element &e,
  const wxString &valueLabel,
  const unsigned objectIndex,
  YAML::Node &yamlMap) const {
  const wxString manualLabel = wxString::Format(WX_P03D, e.manual);

  switch (e.type) {
  case GOCombinationDefinition::COMBINATION_STOP: {
    GOManual &manual = *r_OrganModel.GetManual(e.manual);
    YAML::Node manualNode = yamlMap[MANUALS][manualLabel];

    manualNode[NAME] = manual.GetName();
    manualNode[STOPS][valueLabel] = manual.GetStop(objectIndex)->GetName();
  } break;

  case GOCombinationDefinition::COMBINATION_COUPLER: {
    GOManual &manual = *r_OrganModel.GetManual(e.manual);
    YAML::Node manualNode = yamlMap[MANUALS][manualLabel];

    manualNode[NAME] = manual.GetName();
    manualNode[COUPLERS][valueLabel]
      = manual.GetCoupler(objectIndex)->GetName();
  } break;

  case GOCombinationDefinition::COMBINATION_TREMULANT:
    yamlMap[TREMULANTS][valueLabel]
      = r_OrganModel.GetTremulant(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_SWITCH:
    if (e.manual >= 0) { // manual switch
      GOManual &manual = *r_OrganModel.GetManual(e.manual);
      YAML::Node manualNode = yamlMap[MANUALS][manualLabel];

      manualNode[NAME] = manual.GetName();
      manualNode[SWITCHES][valueLabel]
        = manual.GetSwitch(objectIndex)->GetName();
    } else // global switch
      yamlMap[SWITCHES][valueLabel]
        = r_OrganModel.GetSwitch(objectIndex)->GetName();
    break;

  case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
    yamlMap[DIVISIONAL_COUPLERS][valueLabel]
      = r_OrganModel.GetDivisionalCoupler(objectIndex)->GetName();
    break;
  }
}

void GOGeneralCombination::FromYamlMap(const YAML::Node &yamlMap) {
  // manuals
  const int minManualNum = r_OrganModel.GetFirstManualIndex();
  const int upperManualNum
    = minManualNum + r_OrganModel.GetManualAndPedalCount();

  for (const auto &manualEntry : get_from_map_or_null(yamlMap, MANUALS)) {
    const wxString manualNumStr = manualEntry.first.as<wxString>();
    int manualNum = wxAtoi(manualNumStr);

    if (manualNum >= minManualNum && manualNum < upperManualNum) {
      // stops
      SetStatesFromYaml(
        manualEntry.second[STOPS],
        manualNum,
        GOCombinationDefinition::COMBINATION_STOP);

      // couplers
      SetStatesFromYaml(
        manualEntry.second[COUPLERS],
        manualNum,
        GOCombinationDefinition::COMBINATION_COUPLER);

      // manual switches
      SetStatesFromYaml(
        manualEntry.second[SWITCHES],
        manualNum,
        GOCombinationDefinition::COMBINATION_SWITCH);
    } else
      wxLogError(_("Invalid manual number %s"), manualNumStr);
  }

  // tremulants
  SetStatesFromYaml(
    yamlMap[TREMULANTS], -1, GOCombinationDefinition::COMBINATION_TREMULANT);

  // global switches
  SetStatesFromYaml(
    yamlMap[SWITCHES], -1, GOCombinationDefinition::COMBINATION_SWITCH);

  // divisional couplers
  SetStatesFromYaml(
    yamlMap[DIVISIONAL_COUPLERS],
    -1,
    GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER);
}
