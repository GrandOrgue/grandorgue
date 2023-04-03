/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombination.h"

#include <algorithm>

#include "combinations/GOSetter.h"
#include "model/GODrawStop.h"
#include "yaml/go-wx-yaml.h"

#include "GOCombinationDefinition.h"
#include "GOCombinationElement.h"
#include "GOOrganController.h"

GOCombination::GOCombination(
  GOCombinationDefinition &combination_template,
  GOOrganController *organController)
  : m_OrganFile(organController),
    m_Template(combination_template),
    m_State(0),
    m_Protected(false) {}

GOCombination::~GOCombination() {}

void GOCombination::Clear() {
  UpdateState();
  for (unsigned i = 0; i < m_State.size(); i++)
    m_State[i] = -1;
}

void GOCombination::Copy(GOCombination *combination) {
  assert(GetTemplate() == combination->GetTemplate());
  m_State = combination->m_State;
  UpdateState();
  m_OrganFile->SetOrganModified();
}

bool GOCombination::IsEmpty() const {
  return std::find_if(
           m_State.begin(), m_State.end(), [](int i) { return i > 0; })
    == m_State.end();
}

int GOCombination::GetState(unsigned no) { return m_State[no]; }

void GOCombination::SetState(unsigned no, int value) { m_State[no] = value; }

void GOCombination::GetExtraSetState(
  GOCombination::ExtraElementsSet &extraSet) {
  const std::vector<GOCombinationDefinition::Element> &elements
    = m_Template.GetElements();

  extraSet.clear();
  for (unsigned i = 0; i < elements.size(); i++) {
    if (m_State[i] == 0 && elements[i].control->GetCombinationState())
      extraSet.insert(i);
  }
}

void GOCombination::GetEnabledElements(
  GOCombination::ExtraElementsSet &enabledElements) {
  const std::vector<GOCombinationDefinition::Element> &elements
    = m_Template.GetElements();

  enabledElements.clear();
  for (unsigned i = 0; i < elements.size(); i++) {
    if (m_State[i] > 0)
      enabledElements.insert(i);
  }
}

void GOCombination::UpdateState() {
  const std::vector<GOCombinationDefinition::Element> &elements
    = m_Template.GetElements();
  if (m_State.size() > elements.size())
    m_State.resize(elements.size());
  else if (m_State.size() < elements.size()) {
    unsigned current = m_State.size();
    m_State.resize(elements.size());
    while (current < elements.size())
      m_State[current++] = -1;
  }
}

GOCombinationDefinition *GOCombination::GetTemplate() { return &m_Template; }

bool GOCombination::FillWithCurrent(
  SetterType setterType, bool isToStoreInvisibleObjects) {
  bool used = false;
  const std::vector<GOCombinationDefinition::Element> &elements
    = m_Template.GetElements();

  UpdateState();
  if (setterType == SETTER_REGULAR) {
    for (unsigned i = 0; i < elements.size(); i++) {
      if (!isToStoreInvisibleObjects && !elements[i].store_unconditional)
        m_State[i] = -1;
      else if (elements[i].control->GetCombinationState()) {
        m_State[i] = 1;
        used |= 1;
      } else
        m_State[i] = 0;
    }
  }
  if (setterType == SETTER_SCOPE) {
    for (unsigned i = 0; i < elements.size(); i++) {
      if (!isToStoreInvisibleObjects && !elements[i].store_unconditional)
        m_State[i] = -1;
      else if (elements[i].control->GetCombinationState()) {
        m_State[i] = 1;
        used |= 1;
      } else
        m_State[i] = -1;
    }
  }
  if (setterType == SETTER_SCOPED) {
    for (unsigned i = 0; i < elements.size(); i++) {
      if (m_State[i] != -1) {
        if (elements[i].control->GetCombinationState()) {
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
    const std::vector<GOCombinationDefinition::Element> &elements
      = m_Template.GetElements();

    UpdateState();
    for (unsigned i = 0; i < elements.size(); i++) {
      if (
        m_State[i] != -1
        && (!extraSet || extraSet->find(i) == extraSet->end())) {
        elements[i].control->SetCombination(m_State[i] == 1);
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
