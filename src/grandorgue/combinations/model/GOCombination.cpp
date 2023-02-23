/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombination.h"

#include "combinations/GOSetter.h"
#include "model/GODrawStop.h"

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

int GOCombination::GetState(unsigned no) { return m_State[no]; }

void GOCombination::SetState(unsigned no, int value) { m_State[no] = value; }

void GOCombination::GetExtraSetState(
  GOCombination::ExtraElementsSet &extraSet) {
  const std::vector<GOCombinationDefinition::CombinationSlot> &elements
    = m_Template.GetCombinationElements();

  extraSet.clear();
  for (unsigned i = 0; i < elements.size(); i++) {
    if (m_State[i] == 0 && elements[i].control->GetCombinationState())
      extraSet.insert(i);
  }
}

void GOCombination::GetEnabledElements(
  GOCombination::ExtraElementsSet &enabledElements) {
  const std::vector<GOCombinationDefinition::CombinationSlot> &elements
    = m_Template.GetCombinationElements();

  enabledElements.clear();
  for (unsigned i = 0; i < elements.size(); i++) {
    if (m_State[i] > 0)
      enabledElements.insert(i);
  }
}

void GOCombination::UpdateState() {
  const std::vector<GOCombinationDefinition::CombinationSlot> &elements
    = m_Template.GetCombinationElements();
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

bool GOCombination::PushLocal(GOCombination::ExtraElementsSet const *extraSet) {
  bool used = false;
  const std::vector<GOCombinationDefinition::CombinationSlot> &elements
    = m_Template.GetCombinationElements();
  UpdateState();

  if (m_OrganFile->GetSetter()->IsSetterActive()) {
    if (m_Protected)
      return false;
    if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_REGULAR) {
      for (unsigned i = 0; i < elements.size(); i++) {
        if (
          !m_OrganFile->GetSetter()->StoreInvisibleObjects()
          && !elements[i].store_unconditional)
          m_State[i] = -1;
        else if (elements[i].control->GetCombinationState()) {
          m_State[i] = 1;
          used |= 1;
        } else
          m_State[i] = 0;
      }
      m_OrganFile->SetOrganModified();
    }
    if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_SCOPE) {
      for (unsigned i = 0; i < elements.size(); i++) {
        if (
          !m_OrganFile->GetSetter()->StoreInvisibleObjects()
          && !elements[i].store_unconditional)
          m_State[i] = -1;
        else if (elements[i].control->GetCombinationState()) {
          m_State[i] = 1;
          used |= 1;
        } else
          m_State[i] = -1;
      }
      m_OrganFile->SetOrganModified();
    }
    if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_SCOPED) {
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
  } else {
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
