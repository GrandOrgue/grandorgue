/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombination.h"

#include "GOCombinationDefinition.h"
#include "GOCombinationElement.h"
#include "GODefinitionFile.h"
#include "GODrawStop.h"
#include "GOSetter.h"

GOCombination::GOCombination(GOCombinationDefinition& combination_template,
                             GODefinitionFile* organfile)
    : m_OrganFile(organfile),
      m_Template(combination_template),
      m_State(0),
      m_Protected(false) {}

GOCombination::~GOCombination() {}

void GOCombination::Clear() {
  UpdateState();
  for (unsigned i = 0; i < m_State.size(); i++) m_State[i] = -1;
}

void GOCombination::Copy(GOCombination* combination) {
  assert(GetTemplate() == combination->GetTemplate());
  m_State = combination->m_State;
  UpdateState();
  m_OrganFile->Modified();
}

int GOCombination::GetState(unsigned no) { return m_State[no]; }

void GOCombination::SetState(unsigned no, int value) { m_State[no] = value; }

void GOCombination::UpdateState() {
  const std::vector<GOCombinationDefinition::CombinationSlot>& elements =
      m_Template.GetCombinationElements();
  if (m_State.size() > elements.size())
    m_State.resize(elements.size());
  else if (m_State.size() < elements.size()) {
    unsigned current = m_State.size();
    m_State.resize(elements.size());
    while (current < elements.size()) m_State[current++] = -1;
  }
}

GOCombinationDefinition* GOCombination::GetTemplate() { return &m_Template; }

bool GOCombination::PushLocal() {
  bool used = false;
  const std::vector<GOCombinationDefinition::CombinationSlot>& elements =
      m_Template.GetCombinationElements();
  UpdateState();

  if (m_OrganFile->GetSetter()->IsSetterActive()) {
    if (m_Protected) return false;
    if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_REGULAR) {
      for (unsigned i = 0; i < elements.size(); i++) {
        if (!m_OrganFile->GetSetter()->StoreInvisibleObjects() &&
            !elements[i].store_unconditional)
          m_State[i] = -1;
        else if (elements[i].control->GetCombinationState()) {
          m_State[i] = 1;
          used |= 1;
        } else
          m_State[i] = 0;
      }
      m_OrganFile->Modified();
    }
    if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_SCOPE) {
      for (unsigned i = 0; i < elements.size(); i++) {
        if (!m_OrganFile->GetSetter()->StoreInvisibleObjects() &&
            !elements[i].store_unconditional)
          m_State[i] = -1;
        else if (elements[i].control->GetCombinationState()) {
          m_State[i] = 1;
          used |= 1;
        } else
          m_State[i] = -1;
      }
      m_OrganFile->Modified();
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
      if (m_State[i] != -1) {
        elements[i].control->SetCombination(m_State[i] == 1);
        used |= m_State[i] == 1;
      }
    }
  }

  return used;
}
