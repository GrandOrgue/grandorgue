/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombinationDefinition.h"

#include <wx/intl.h>

#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

const wxString GOCombinationDefinition::ELEMENT_TYPE_NAMES[] = {
  _("stop"),
  _("coupler"),
  _("tremulant"),
  _("divisional coupler"),
  _("switch")};

GOCombinationDefinition::GOCombinationDefinition(GOOrganModel &organModel)
  : r_OrganModel(organModel), m_elements(0) {}

GOCombinationDefinition::~GOCombinationDefinition() {}

void GOCombinationDefinition::AddGeneral(
  GODrawstop *control, ElementType type, int manual, unsigned index) {
  Add(control, type, manual, index, control->IsToStoreInGeneral());
}

void GOCombinationDefinition::AddDivisional(
  GODrawstop *control, ElementType type, int manual, unsigned index) {
  Add(control, type, manual, index, control->IsToStoreInDivisional());
}

void GOCombinationDefinition::Add(
  GODrawstop *control,
  ElementType type,
  int manual,
  unsigned index,
  bool store_unconditional) {
  if (control->IsReadOnly())
    return;
  Element def;
  def.type = type;
  def.manual = manual;
  def.index = index;
  def.store_unconditional = store_unconditional;
  def.control = control;
  if (manual == -1)
    def.group = wxEmptyString;
  else
    def.group = r_OrganModel.GetManual(manual)->GetName();
  m_elements.push_back(def);
}

void GOCombinationDefinition::InitGeneral() {
  m_elements.resize(0);

  for (unsigned j = r_OrganModel.GetFirstManualIndex();
       j <= r_OrganModel.GetManualAndPedalCount();
       j++) {
    for (unsigned i = 0; i < r_OrganModel.GetManual(j)->GetStopCount(); i++)
      AddGeneral(
        r_OrganModel.GetManual(j)->GetStop(i), COMBINATION_STOP, j, i + 1);
  }

  for (unsigned j = r_OrganModel.GetFirstManualIndex();
       j <= r_OrganModel.GetManualAndPedalCount();
       j++) {
    for (unsigned i = 0; i < r_OrganModel.GetManual(j)->GetCouplerCount(); i++)
      AddGeneral(
        r_OrganModel.GetManual(j)->GetCoupler(i),
        COMBINATION_COUPLER,
        j,
        i + 1);
  }

  for (unsigned i = 0; i < r_OrganModel.GetTremulantCount(); i++)
    AddGeneral(r_OrganModel.GetTremulant(i), COMBINATION_TREMULANT, -1, i + 1);

  for (unsigned i = 0; i < r_OrganModel.GetSwitchCount(); i++)
    AddGeneral(r_OrganModel.GetSwitch(i), COMBINATION_SWITCH, -1, i + 1);

  for (unsigned i = 0; i < r_OrganModel.GetDivisionalCouplerCount(); i++)
    AddGeneral(
      r_OrganModel.GetDivisionalCoupler(i),
      COMBINATION_DIVISIONALCOUPLER,
      -1,
      i + 1);
}

void GOCombinationDefinition::InitDivisional(unsigned manual_number) {
  GOManual *associatedManual = r_OrganModel.GetManual(manual_number);
  m_elements.resize(0);

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

int GOCombinationDefinition::FindElement(
  ElementType type, int manual, unsigned index) const {
  for (unsigned i = 0; i < m_elements.size(); i++) {
    if (
      m_elements[i].type == type && m_elements[i].manual == manual
      && m_elements[i].index == index)
      return i;
  }
  return -1;
}
