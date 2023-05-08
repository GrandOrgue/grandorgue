/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalButtonControl.h"

#include "wx/intl.h"

#include "combinations/GOSetter.h"
#include "model/GOManual.h"

#include "GOOrganController.h"

GODivisionalButtonControl::GODivisionalButtonControl(
  GOOrganController *organController,
  GOCombinationDefinition &divisionalTemplate,
  bool isSetter)
  : GOPushbuttonControl(organController),
    r_setter(*organController->GetSetter()),
    m_combination(organController, divisionalTemplate, isSetter) {}

wxString GODivisionalButtonControl::GetMidiType() { return _("Divisional"); };

void GODivisionalButtonControl::Init(
  GOConfigReader &cfg,
  const wxString &group,
  int manualNumber,
  int divisionalNumber,
  const wxString &name) {
  GOPushbuttonControl::Init(cfg, group, name);
  m_combination.Init(group, manualNumber, divisionalNumber);
}
void GODivisionalButtonControl::Load(
  GOConfigReader &cfg,
  const wxString &group,
  int manualNumber,
  int divisionalNumber) {
  GOPushbuttonControl::Load(cfg, group);
  m_combination.Load(cfg, group, manualNumber, divisionalNumber);
}

void GODivisionalButtonControl::LoadCombination(GOConfigReader &cfg) {
  m_combination.LoadCombination(cfg);
}

void GODivisionalButtonControl::Save(GOConfigWriter &cfg) {
  GOPushbuttonControl::Save(cfg);
  m_combination.Save(cfg);
}

void GODivisionalButtonControl::Push() {
  GOCombination::ExtraElementsSet elementSet;
  const GOCombination::ExtraElementsSet *pAddSet
    = r_setter.GetCrescendoAddSet(elementSet);
  GOManual *pManual
    = m_OrganController->GetManual(m_combination.GetManualNumber());

  r_setter.NotifyCmbPushed(m_combination.Push(pAddSet));

  for (unsigned l = pManual->GetDivisionalCount(), k = 0; k < l; k++) {
    GODivisionalButtonControl *pDivisionalControl = pManual->GetDivisional(k);

    pDivisionalControl->Display(pDivisionalControl == this);
  }
}
