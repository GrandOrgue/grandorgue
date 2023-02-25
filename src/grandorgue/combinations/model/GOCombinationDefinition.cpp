/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombinationDefinition.h"

#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

#include "GOOrganController.h"

GOCombinationDefinition::GOCombinationDefinition(
  GOOrganController *organController)
  : m_OrganController(organController), m_Content(0) {}

GOCombinationDefinition::~GOCombinationDefinition() {}

void GOCombinationDefinition::AddGeneral(
  GODrawstop *control, CombinationType type, int manual, unsigned index) {
  Add(control, type, manual, index, control->GetStoreGeneral());
}

void GOCombinationDefinition::AddDivisional(
  GODrawstop *control, CombinationType type, int manual, unsigned index) {
  Add(control, type, manual, index, control->GetStoreDivisional());
}

void GOCombinationDefinition::Add(
  GODrawstop *control,
  CombinationType type,
  int manual,
  unsigned index,
  bool store_unconditional) {
  if (control->IsReadOnly())
    return;
  CombinationSlot def;
  def.type = type;
  def.manual = manual;
  def.index = index;
  def.store_unconditional = store_unconditional;
  def.control = control;
  if (manual == -1)
    def.group = wxEmptyString;
  else
    def.group = m_OrganController->GetManual(manual)->GetName();
  m_Content.push_back(def);
}

void GOCombinationDefinition::InitGeneral() {
  m_Content.resize(0);

  for (unsigned j = m_OrganController->GetFirstManualIndex();
       j <= m_OrganController->GetManualAndPedalCount();
       j++) {
    for (unsigned i = 0; i < m_OrganController->GetManual(j)->GetStopCount();
         i++)
      AddGeneral(
        m_OrganController->GetManual(j)->GetStop(i),
        COMBINATION_STOP,
        j,
        i + 1);
  }

  for (unsigned j = m_OrganController->GetFirstManualIndex();
       j <= m_OrganController->GetManualAndPedalCount();
       j++) {
    for (unsigned i = 0; i < m_OrganController->GetManual(j)->GetCouplerCount();
         i++)
      AddGeneral(
        m_OrganController->GetManual(j)->GetCoupler(i),
        COMBINATION_COUPLER,
        j,
        i + 1);
  }

  for (unsigned i = 0; i < m_OrganController->GetTremulantCount(); i++)
    AddGeneral(
      m_OrganController->GetTremulant(i), COMBINATION_TREMULANT, -1, i + 1);

  for (unsigned i = 0; i < m_OrganController->GetSwitchCount(); i++)
    AddGeneral(m_OrganController->GetSwitch(i), COMBINATION_SWITCH, -1, i + 1);

  for (unsigned i = 0; i < m_OrganController->GetDivisionalCouplerCount(); i++)
    AddGeneral(
      m_OrganController->GetDivisionalCoupler(i),
      COMBINATION_DIVISIONALCOUPLER,
      -1,
      i + 1);
}

void GOCombinationDefinition::InitDivisional(unsigned manual_number) {
  GOManual *associatedManual = m_OrganController->GetManual(manual_number);
  m_Content.resize(0);

  for (unsigned i = 0; i < associatedManual->GetStopCount(); i++)
    AddDivisional(
      associatedManual->GetStop(i), COMBINATION_STOP, manual_number, i + 1);

  for (unsigned i = 0; i < associatedManual->GetCouplerCount(); i++)
    AddDivisional(
      associatedManual->GetCoupler(i),
      COMBINATION_COUPLER,
      manual_number,
      i + 1);

  for (unsigned i = 0; i < associatedManual->GetTremulantCount(); i++)
    AddDivisional(
      associatedManual->GetTremulant(i),
      COMBINATION_TREMULANT,
      manual_number,
      i + 1);

  for (unsigned i = 0; i < associatedManual->GetSwitchCount(); i++)
    AddDivisional(
      associatedManual->GetSwitch(i), COMBINATION_SWITCH, manual_number, i + 1);
}

const std::vector<GOCombinationDefinition::CombinationSlot>
  &GOCombinationDefinition::GetCombinationElements() {
  return m_Content;
}

int GOCombinationDefinition::findEntry(
  CombinationType type, int manual, unsigned index) {
  for (unsigned i = 0; i < m_Content.size(); i++) {
    if (
      m_Content[i].type == type && m_Content[i].manual == manual
      && m_Content[i].index == index)
      return i;
  }
  return -1;
}
