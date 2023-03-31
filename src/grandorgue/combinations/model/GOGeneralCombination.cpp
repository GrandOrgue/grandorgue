/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGeneralCombination.h"

#include <wx/intl.h>
#include <wx/log.h>
#include <yaml-cpp/yaml.h>

#include "combinations/GOSetter.h"
#include "combinations/control/GODivisionalButtonControl.h"
#include "combinations/control/GOGeneralButtonControl.h"
#include "config/GOConfigWriter.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "yaml/go-wx-yaml.h"

#include "GOOrganController.h"

GOGeneralCombination::GOGeneralCombination(
  GOCombinationDefinition &general_template,
  GOOrganController *organController,
  bool is_setter)
  : GOCombination(general_template, organController),
    m_OrganController(organController),
    m_IsSetter(is_setter) {}

void GOGeneralCombination::Load(GOConfigReader &cfg, wxString group) {
  m_OrganController->RegisterSaveableObject(this);
  m_group = group;

  m_Protected
    = cfg.ReadBoolean(ODFSetting, group, wxT("Protected"), false, false);

  /* check ODF settings */
  if (!m_IsSetter)
    LoadCombinationInt(cfg, ODFSetting);
}

void GOGeneralCombination::LoadCombinationInt(
  GOConfigReader &cfg, GOSettingType srcType) {
  wxString buffer;
  unsigned NumberOfStops
    = ReadNumberOfStops(cfg, srcType, m_OrganController->GetStopCount());
  unsigned NumberOfCouplers = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfCouplers"),
    0,
    srcType == CMBSetting ? m_OrganController->GetCouplerCount()
                          : m_OrganController->GetODFCouplerCount());
  unsigned NumberOfTremulants = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    m_OrganController->GetTremulantCount());
  unsigned NumberOfSwitches = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    m_OrganController->GetSwitchCount(),
    false,
    0);
  unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
    srcType,
    m_group,
    wxT("NumberOfDivisionalCouplers"),
    0,
    m_OrganController->GetDivisionalCouplerCount(),
    m_OrganController->GeneralsStoreDivisionalCouplers());

  Clear();

  for (unsigned i = 0; i < NumberOfStops; i++) {
    unsigned m = cfg.ReadInteger(
      srcType,
      m_group,
      wxString::Format(wxT("StopManual%03d"), i + 1),
      m_OrganController->GetFirstManualIndex(),
      m_OrganController->GetManualAndPedalCount());

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
      m_OrganController->GetFirstManualIndex(),
      m_OrganController->GetManualAndPedalCount());
    unsigned cnt = srcType == CMBSetting
      ? m_OrganController->GetManual(m)->GetCouplerCount()
      : m_OrganController->GetManual(m)->GetODFCouplerCount();

    buffer.Printf(wxT("CouplerNumber%03d"), i + 1);
    SetLoadedState(
      m,
      GOCombinationDefinition::COMBINATION_COUPLER,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  unsigned cnt = m_OrganController->GetTremulantCount();

  for (unsigned i = 0; i < NumberOfTremulants; i++) {
    buffer.Printf(wxT("TremulantNumber%03d"), i + 1);
    SetLoadedState(
      -1,
      GOCombinationDefinition::COMBINATION_TREMULANT,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  cnt = m_OrganController->GetSwitchCount();
  for (unsigned i = 0; i < NumberOfSwitches; i++) {
    buffer.Printf(wxT("SwitchNumber%03d"), i + 1);
    SetLoadedState(
      -1,
      GOCombinationDefinition::COMBINATION_SWITCH,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }

  cnt = m_OrganController->GetDivisionalCouplerCount();
  for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++) {
    buffer.Printf(wxT("DivisionalCouplerNumber%03d"), i + 1);
    SetLoadedState(
      -1,
      GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER,
      cfg.ReadInteger(srcType, m_group, buffer, -cnt, cnt),
      buffer);
  }
}

void GOGeneralCombination::Push(
  ExtraElementsSet const *extraSet, bool isFromCrescendo) {
  GOCombination::PushLocal(extraSet);

  if (!isFromCrescendo || !extraSet) { // Otherwise the crescendo in add mode:
                                       // not to switch off combination buttons
    m_OrganController->GetSetter()->ResetDisplay(); // disable buttons

    for (unsigned k = 0; k < m_OrganController->GetGeneralCount(); k++) {
      GOGeneralButtonControl *general = m_OrganController->GetGeneral(k);
      general->Display(&general->GetCombination() == this);
    }

    for (unsigned j = m_OrganController->GetFirstManualIndex();
         j <= m_OrganController->GetManualAndPedalCount();
         j++) {
      for (unsigned k = 0;
           k < m_OrganController->GetManual(j)->GetDivisionalCount();
           k++)
        m_OrganController->GetManual(j)->GetDivisional(k)->Display(false);
    }
  }
}

void GOGeneralCombination::Save(GOConfigWriter &cfg) {
  UpdateState();

  wxString buffer;
  unsigned stop_count = 0;
  unsigned coupler_count = 0;
  unsigned tremulant_count = 0;
  unsigned switch_count = 0;
  unsigned divisional_coupler_count = 0;

  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
    const GOCombinationDefinition::Element &e = r_ElementDefinitions[i];
    int state = GetState(i);

    if (state >= 0) {
      int value = state == 1 ? e.index : -e.index;
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
        buffer.Printf(wxT("SwitchNumber%03d"), switch_count);
        cfg.WriteInteger(m_group, buffer, value);
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

void GOGeneralCombination::ToYaml(YAML::Node &yamlNode) const {
  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++)
    if (GetState(i) > 0) {
      const GOCombinationDefinition::Element &e = r_ElementDefinitions[i];
      unsigned value = e.index;
      int manualIndex = e.manual;
      const wxString manualLabel = wxString::Format(WX_P03D, manualIndex);
      const wxString valueLabel = wxString::Format(WX_P03D, value);

      assert(value > 0);

      unsigned index = value - 1;

      switch (e.type) {
      case GOCombinationDefinition::COMBINATION_STOP: {
        GOManual &manual = *m_OrganController->GetManual(manualIndex);
        YAML::Node manualNode = yamlNode[MANUALS][manualLabel];

        manualNode[NAME] = manual.GetName();
        manualNode[STOPS][valueLabel] = manual.GetStop(index)->GetName();
      } break;

      case GOCombinationDefinition::COMBINATION_COUPLER: {
        GOManual &manual = *m_OrganController->GetManual(manualIndex);
        YAML::Node manualNode = yamlNode[MANUALS][manualLabel];

        manualNode[NAME] = manual.GetName();
        manualNode[COUPLERS][valueLabel] = manual.GetCoupler(index)->GetName();
      } break;

      case GOCombinationDefinition::COMBINATION_TREMULANT:
        yamlNode[TREMULANTS][valueLabel]
          = m_OrganController->GetTremulant(index)->GetName();
        break;

      case GOCombinationDefinition::COMBINATION_SWITCH:
        yamlNode[SWITCHES][valueLabel]
          = m_OrganController->GetSwitch(index)->GetName();
        break;

      case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
        yamlNode[DIVISIONAL_COUPLERS][valueLabel]
          = m_OrganController->GetDivisionalCoupler(index)->GetName();
        break;
      }
    }
}

void GOGeneralCombination::FromYaml(const YAML::Node &node) {
  throw wxT("Not implemented yet");
}
