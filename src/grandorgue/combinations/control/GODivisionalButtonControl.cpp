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
    m_divisional(organController, divisionalTemplate, isSetter) {}

wxString GODivisionalButtonControl::GetMidiType() { return _("Divisional"); };

void GODivisionalButtonControl::Init(
  GOConfigReader &cfg,
  const wxString &group,
  int manualNumber,
  int divisionalNumber,
  const wxString &name) {
  GOPushbuttonControl::Init(cfg, group, name);
  m_divisional.Init(group, manualNumber, divisionalNumber);
}
void GODivisionalButtonControl::Load(
  GOConfigReader &cfg,
  const wxString &group,
  int manualNumber,
  int divisionalNumber) {
  GOPushbuttonControl::Load(cfg, group);
  m_divisional.Load(cfg, group, manualNumber, divisionalNumber);
}

void GODivisionalButtonControl::LoadCombination(GOConfigReader &cfg) {
  m_divisional.LoadCombination(cfg);
}

void GODivisionalButtonControl::Save(GOConfigWriter &cfg) {
  GOPushbuttonControl::Save(cfg);
  m_divisional.Save(cfg);
}

void GODivisionalButtonControl::Push() {
  GOCombination::ExtraElementsSet elementSet;
  const GOCombination::ExtraElementsSet *pAddSet
    = m_OrganController->GetSetter()->GetCrescendoAddSet(elementSet);
  GOManual *pManual
    = m_OrganController->GetManual(m_divisional.GetManualNumber());

  m_divisional.Push(pAddSet);

  for (unsigned l = pManual->GetDivisionalCount(), k = 0; k < l; k++) {
    GODivisionalButtonControl *pDivisionalControl = pManual->GetDivisional(k);

    pDivisionalControl->Display(pDivisionalControl == this);
  }
}
