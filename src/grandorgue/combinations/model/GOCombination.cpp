/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombination.h"

#include <algorithm>

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfigWriter.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GODrawStop.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "yaml/go-wx-yaml.h"

#include "GOCombinationDefinition.h"
#include "GOCombinationElement.h"

class GOOrganModel;

GOCombination::GOCombination(
  GOOrganModel &organModel, const GOCombinationDefinition &cmbDef)
  : r_OrganModel(organModel),
    m_Template(cmbDef),
    m_IsFull(false),
    m_HasScope(false),
    r_ElementDefinitions(cmbDef.GetElements()),
    m_Protected(false) {}

GOCombination::~GOCombination() {}

void GOCombination::Clear() {
  UpdateState();
  for (unsigned i = 0; i < m_State.size(); i++)
    m_State[i] = -1;
  m_IsFull = false;
  m_HasScope = false;
}

void GOCombination::Copy(GOCombination *combination) {
  assert(&m_Template == &combination->m_Template);
  m_State = combination->m_State;
  UpdateState();
}

bool GOCombination::IsEmpty() const {
  return std::find_if(
           m_State.begin(), m_State.end(), [](int i) { return i >= 0; })
    == m_State.end();
}

void GOCombination::SetLoadedState(
  int manualNumber,
  GOCombinationDefinition::ElementType elementType,
  int elementNumber,
  const wxString &elementName) {
  int pos
    = m_Template.FindElement(elementType, manualNumber, abs(elementNumber));

  if (pos >= 0) {
    int &state = m_State[pos];

    // when loading scope/scoped cmb from yaml, the same element may be read
    // twice: for the scope and for the regular cmb
    if (state < 0 || (m_HasScope && state == 0)) // has not yet been set
      state = (elementNumber > 0) ? 1 : 0;
    else // has already been set
      wxLogError(
        _("Duplicate combination entry %s in %s"), elementName, m_group);
  } else
    wxLogError(_("Invalid combination entry %s in %s"), elementName, m_group);
}

void GOCombination::SetStatesFromYaml(
  const YAML::Node &yamlNode,
  int manualNumber,
  GOCombinationDefinition::ElementType elementType) {
  if (yamlNode.IsDefined() && yamlNode.IsMap()) {
    const wxString &elementTypeName
      = GOCombinationDefinition::ELEMENT_TYPE_NAMES[elementType];
    GOManual *pManual
      = manualNumber >= 0 ? r_OrganModel.GetManual(manualNumber) : nullptr;
    int maxElementNumber = 0;

    // find maxElementNumber dependent on the element type
    switch (elementType) {
    case GOCombinationDefinition::COMBINATION_STOP:
      if (pManual)
        maxElementNumber = pManual->GetStopCount();
      break;
    case GOCombinationDefinition::COMBINATION_COUPLER:
      if (pManual)
        maxElementNumber = pManual->GetCouplerCount();
      break;
    case GOCombinationDefinition::COMBINATION_TREMULANT:
      maxElementNumber = r_OrganModel.GetTremulantCount();
      break;
    case GOCombinationDefinition::COMBINATION_SWITCH:
      maxElementNumber = r_OrganModel.GetSwitchCount();
      break;
    case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
      maxElementNumber = r_OrganModel.GetDivisionalCouplerCount();
      break;
    }

    for (const auto &entry : yamlNode) {
      const wxString numStr = entry.first.as<wxString>();
      wxString name = entry.second.as<wxString>();
      int numFromYaml = wxAtoi(numStr);
      wxString
        realElementName; // the name of the object referenced with numfromYaml

      if (numFromYaml > 0 && numFromYaml <= maxElementNumber) {
        unsigned i = (unsigned)(numFromYaml - 1);

        switch (elementType) {
        case GOCombinationDefinition::COMBINATION_STOP:
          if (pManual)
            realElementName = pManual->GetStop(i)->GetName();
          break;
        case GOCombinationDefinition::COMBINATION_COUPLER:
          if (pManual)
            realElementName = pManual->GetCoupler(i)->GetName();
          break;
        case GOCombinationDefinition::COMBINATION_TREMULANT:
          realElementName = r_OrganModel.GetTremulant(i)->GetName();
          break;
        case GOCombinationDefinition::COMBINATION_SWITCH:
          realElementName = r_OrganModel.GetSwitch(i)->GetName();
          break;
        case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
          realElementName = r_OrganModel.GetDivisionalCoupler(i)->GetName();
          break;
        }
      } else
        numFromYaml = 0;      // invalid element
      unsigned fitNumber = 0; // the number of matched element (from 1 to
                              // maxElementNumber). 0 - not matched

      // validate the name
      if (realElementName == name)
        fitNumber = numFromYaml; // everything matches
      else {
        // names differ. Find the object by name
        int i = -1;

        switch (elementType) {
        case GOCombinationDefinition::COMBINATION_STOP:
          if (pManual)
            i = pManual->FindStopByName(name);
          break;
        case GOCombinationDefinition::COMBINATION_COUPLER:
          if (pManual)
            i = pManual->FindCouplerByName(name);
          break;
        case GOCombinationDefinition::COMBINATION_TREMULANT:
          i = pManual ? pManual->FindTremulantByName(name)
                      : r_OrganModel.FindTremulantByName(name);
          break;
        case GOCombinationDefinition::COMBINATION_SWITCH:
          i = pManual ? pManual->FindSwitchByName(name)
                      : r_OrganModel.FindSwitchByName(name);
          break;
        case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
          i = r_OrganModel.FindDivisionalCouplerByName(name);
          break;
        }
        fitNumber = (unsigned)(i + 1);

        if (fitNumber > 0) // matched by name
          wxLogWarning(
            _("Wrong number %s of the %s \"%s\""),
            numStr,
            elementTypeName,
            name);
        else if (numFromYaml > 0) { // matched by number
          wxLogWarning(
            _("Wrong name \"%s\" instead of \"%s\" of the %s %s"),
            name,
            realElementName,
            elementTypeName,
            numStr);
          fitNumber = (unsigned)numFromYaml;
          name = realElementName;
        }
      }
      if (fitNumber) // matched
        SetLoadedState(manualNumber, elementType, fitNumber, name);
      else
        wxLogError(
          _("Could match the %s \"%s: %s\" neither by name nor by "
            "number"),
          elementTypeName,
          numStr,
          name);
    }
  }
}

void GOCombination::GetExtraSetState(
  GOCombination::ExtraElementsSet &extraSet) {
  extraSet.clear();
  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
    if (
      m_State[i] == 0 && r_ElementDefinitions[i].control->GetCombinationState())
      extraSet.insert(i);
  }
}

void GOCombination::GetEnabledElements(
  GOCombination::ExtraElementsSet &enabledElements) {
  enabledElements.clear();
  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
    if (m_State[i] > 0)
      enabledElements.insert(i);
  }
}

void GOCombination::UpdateState() {
  unsigned defSize = r_ElementDefinitions.size();

  if (m_State.size() > defSize)
    m_State.resize(defSize);
  else if (m_State.size() < defSize) {
    unsigned current = m_State.size();
    m_State.resize(defSize);
    while (current < defSize)
      m_State[current++] = -1;
  }
}

static const wxString WX_NUMBER_OF_STOPS = wxT("NumberOfStops");

int read_number_of_stops(
  GOSettingType settingType,
  GOConfigReader &cfg,
  const wxString &group,
  unsigned maxStopN,
  bool isRequired) {
  return cfg.ReadInteger(
    settingType,
    group,
    WX_NUMBER_OF_STOPS,
    0,
    maxStopN,
    isRequired,
    isRequired ? 0 : -1);
}

// checks if a combinatiom exists in the file with the group
bool is_cmb_on_file(
  GOConfigReader &cfg, GOSettingType settingType, const wxString &group) {
  int nOfStops = read_number_of_stops(settingType, cfg, group, 999, false);

  return nOfStops >= 0;
}

bool GOCombination::isCmbOnFile(GOConfigReader &cfg, const wxString &group) {
  return is_cmb_on_file(cfg, ODFSetting, group)
    || is_cmb_on_file(cfg, CMBSetting, group);
}

unsigned GOCombination::ReadNumberOfStops(
  GOConfigReader &cfg, GOSettingType srcType, unsigned maxStops) const {
  int nOfStopsInt = read_number_of_stops(srcType, cfg, m_group, maxStops, true);

  assert(nOfStopsInt >= 0);
  return (unsigned)nOfStopsInt;
}

void GOCombination::WriteNumberOfStops(
  GOConfigWriter &cfg, unsigned stopCount) const {
  cfg.WriteInteger(m_group, WX_NUMBER_OF_STOPS, stopCount);
}

const wxString WX_IS_FULL = wxT("IsFull");
const wxString WX_HAS_SCOPE = wxT("HasScope");

// Load the combination either from the odf or from the cmb
void GOCombination::LoadCombination(
  GOConfigReader &cfg, GOSettingType srcType) {
  Clear();
  m_IsFull = cfg.ReadBoolean(srcType, m_group, WX_IS_FULL, false, true);
  LoadCombinationInt(cfg, srcType);
  m_HasScope = cfg.ReadBoolean(srcType, m_group, WX_HAS_SCOPE, false, false);
}

void GOCombination::LoadCombination(GOConfigReader &cfg) {
  Clear();

  if (is_cmb_on_file(cfg, CMBSetting, m_group))
    LoadCombination(cfg, CMBSetting);
  else if (is_cmb_on_file(cfg, ODFSetting, m_group))
    LoadCombination(cfg, ODFSetting);
}

void GOCombination::Save(GOConfigWriter &cfg) {
  cfg.WriteBoolean(m_group, WX_IS_FULL, m_IsFull);
  cfg.WriteBoolean(m_group, WX_HAS_SCOPE, m_HasScope);
  SaveInt(cfg);
}

const wxString WX_P03D = wxT("%03d");
const char *const FULL = "full";
const char *const SCOPE = "scope";

void GOCombination::PutElementsToYaml(
  YAML::Node &yamlMap, int stateFrom) const {
  for (unsigned i = 0; i < r_ElementDefinitions.size(); i++)
    if (GetState(i) >= stateFrom) {
      const auto &e = r_ElementDefinitions[i];
      unsigned value = e.index;

      assert(value > 0);
      PutElementToYamlMap(
        e, wxString::Format(WX_P03D, value), value - 1, yamlMap);
    }
}

void GOCombination::ToYaml(YAML::Node &yamlMap) const {
  PutElementsToYaml(yamlMap, 1);
  if (m_HasScope) {
    YAML::Node scopeNode(YAML::NodeType::Map);

    PutElementsToYaml(scopeNode, 0);
    yamlMap[SCOPE] = scopeNode;
  }
  if (m_IsFull)
    yamlMap[FULL] = true; // save whether the combination is set as full
}

void GOCombination::FromYaml(const YAML::Node &yamlNode) {
  if (!m_Protected) {
    Clear();

    if (yamlNode.IsDefined() && yamlNode.IsMap()) {
      const YAML::Node scopeNode = yamlNode[SCOPE];

      m_HasScope = scopeNode.IsDefined();

      if (m_HasScope) {
        // set state to 0 for the scope
        FromYamlMap(scopeNode);
        std::replace(m_State.begin(), m_State.end(), 1, 0);
      }

      FromYamlMap(yamlNode);
      m_IsFull = yamlNode[FULL].as<bool>(false);

      // clear all non mentioned elements. Otherwise they won't be disabled when
      // this combination is switched on
      if (!m_HasScope) {
        for (unsigned l = r_ElementDefinitions.size(), i = 0; i < l; i++) {
          auto &state = m_State[i];

          if (
            state < 0
            && (r_ElementDefinitions[i].store_unconditional || m_IsFull))
            state = 0;
          // else the element is not visible so it shouldn't be touched by the
          // combination
        }
      }
    }
  }
}

bool GOCombination::FillWithCurrent(
  GOSetterState::SetterType setterType, bool isToStoreInvisibleObjects) {
  bool used = false;

  UpdateState();
  m_IsFull = isToStoreInvisibleObjects;
  switch (setterType) {
  case GOSetterState::SETTER_REGULAR:
    m_HasScope = false;
    for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
      if (
        !isToStoreInvisibleObjects
        && !r_ElementDefinitions[i].store_unconditional)
        m_State[i] = -1;
      else if (r_ElementDefinitions[i].control->GetCombinationState()) {
        m_State[i] = 1;
        used |= 1;
      } else
        m_State[i] = 0;
    }
    break;
  case GOSetterState::SETTER_SCOPE:
    m_HasScope = true;
    for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
      if (
        !isToStoreInvisibleObjects
        && !r_ElementDefinitions[i].store_unconditional)
        m_State[i] = -1;
      else if (r_ElementDefinitions[i].control->GetCombinationState()) {
        m_State[i] = 1;
        used |= 1;
      } else
        m_State[i] = -1;
    }
    break;
  case GOSetterState::SETTER_SCOPED:
    m_HasScope = true;
    for (unsigned i = 0; i < r_ElementDefinitions.size(); i++)
      if (m_State[i] != -1) {
        if (r_ElementDefinitions[i].control->GetCombinationState()) {
          m_State[i] = 1;
          used |= 1;
        } else
          m_State[i] = 0;
      }
    break;
  }
  return used;
}

bool GOCombination::Push(
  const GOSetterState &setterState,
  const GOCombination::ExtraElementsSet *extraSet) {
  bool used = false;

  if (setterState.m_IsActive) {
    if (!m_Protected) {
      used = FillWithCurrent(
        setterState.m_SetterType, setterState.m_IsStoreInvisible);
    }
  } else {
    UpdateState();
    for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
      if (
        m_State[i] != -1
        && (!extraSet || extraSet->find(i) == extraSet->end())) {
        r_ElementDefinitions[i].control->SetCombination(m_State[i] == 1);
        used |= m_State[i] == 1;
      }
    }
  }

  return used;
}

void GOCombination::PutToYamlMap(YAML::Node &container, const char *key) const {
  if (!IsEmpty())
    container[key] = ToYamlNode();
}

void GOCombination::putToYamlMap(
  YAML::Node &container, const wxString &key, const GOCombination *pCmb) {
  if (pCmb)
    pCmb->PutToYamlMap(container, key);
}
