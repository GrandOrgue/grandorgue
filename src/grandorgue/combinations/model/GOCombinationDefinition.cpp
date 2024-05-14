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

  for (unsigned i = 0; i < r_OrganModel.GetSwitchCount(); i++) {
    GOSwitch *pSwitch = r_OrganModel.GetSwitch(i);
    int manualN = pSwitch->GetAssociatedManualN();

    AddGeneral(
      pSwitch,
      COMBINATION_SWITCH,
      manualN,
      (manualN >= 0 ? pSwitch->GetIndexInManual() : i) + 1);
  }

  for (unsigned i = 0; i < r_OrganModel.GetDivisionalCouplerCount(); i++)
    AddGeneral(
      r_OrganModel.GetDivisionalCoupler(i),
      COMBINATION_DIVISIONALCOUPLER,
      -1,
      i + 1);
}

void GOCombinationDefinition::InitDivisional(GOManual &manual) {
  unsigned manualN = manual.GetManulNumber();

  m_elements.resize(0);

  for (unsigned i = 0; i < manual.GetStopCount(); i++)
    AddDivisional(manual.GetStop(i), COMBINATION_STOP, manualN, i + 1);

  for (unsigned i = 0; i < manual.GetCouplerCount(); i++)
    AddDivisional(manual.GetCoupler(i), COMBINATION_COUPLER, manualN, i + 1);

  for (unsigned i = 0; i < manual.GetTremulantCount(); i++)
    AddDivisional(
      manual.GetTremulant(i), COMBINATION_TREMULANT, manualN, i + 1);

  for (unsigned i = 0; i < manual.GetSwitchCount(); i++)
    AddDivisional(manual.GetSwitch(i), COMBINATION_SWITCH, manualN, i + 1);
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
