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

#include "combinations/GOSetter.h"
#include "config/GOConfigWriter.h"
#include "model/GODrawStop.h"
#include "yaml/go-wx-yaml.h"

#include "GOCombinationDefinition.h"
#include "GOCombinationElement.h"
#include "GOOrganController.h"

GOCombination::GOCombination(
  const GOCombinationDefinition &combination_template,
  GOOrganController *organController)
  : m_Template(combination_template),
    m_OrganFile(organController),
    r_ElementDefinitions(combination_template.GetElements()),
    m_Protected(false) {}

GOCombination::~GOCombination() {}

void GOCombination::Clear() {
  UpdateState();
  for (unsigned i = 0; i < m_State.size(); i++)
    m_State[i] = -1;
}

void GOCombination::Copy(GOCombination *combination) {
  assert(&m_Template == &combination->m_Template);
  m_State = combination->m_State;
  UpdateState();
  m_OrganFile->SetOrganModified();
}

bool GOCombination::IsEmpty() const {
  return std::find_if(
           m_State.begin(), m_State.end(), [](int i) { return i > 0; })
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

    if (state < 0) // has not yet been set
      state = (elementNumber > 0) ? 1 : 0;
    else // has already been set
      wxLogError(
        _("Duplicate combination entry %s in %s"), elementName, m_group);
  } else
    wxLogError(_("Invalid combination entry %s in %s"), elementName, m_group);
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

void GOCombination::LoadCombination(GOConfigReader &cfg) {
  if (is_cmb_on_file(cfg, CMBSetting, m_group))
    LoadCombinationInt(cfg, CMBSetting);
  else if (is_cmb_on_file(cfg, ODFSetting, m_group))
    LoadCombinationInt(cfg, ODFSetting);
  else
    Clear();
}

bool GOCombination::FillWithCurrent(
  SetterType setterType, bool isToStoreInvisibleObjects) {
  bool used = false;

  UpdateState();
  if (setterType == SETTER_REGULAR) {
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
  }
  if (setterType == SETTER_SCOPE) {
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
  }
  if (setterType == SETTER_SCOPED) {
    for (unsigned i = 0; i < r_ElementDefinitions.size(); i++) {
      if (m_State[i] != -1) {
        if (r_ElementDefinitions[i].control->GetCombinationState()) {
          m_State[i] = 1;
          used |= 1;
        } else
          m_State[i] = 0;
      }
    }
  }
  return used;
}

bool GOCombination::PushLocal(GOCombination::ExtraElementsSet const *extraSet) {
  bool used = false;
  GOSetter &setter = *m_OrganFile->GetSetter();

  if (setter.IsSetterActive()) {
    if (!m_Protected) {

      used = FillWithCurrent(
        setter.GetSetterType(), setter.StoreInvisibleObjects());
      m_OrganFile->SetOrganModified();
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
    put_to_map_if_not_null(container, key, ToYamlNode());
}

void GOCombination::putToYamlMap(
  YAML::Node &container, const wxString &key, const GOCombination *pCmb) {
  if (pCmb)
    pCmb->PutToYamlMap(container, key);
}
