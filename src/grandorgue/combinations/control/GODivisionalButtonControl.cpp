/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalButtonControl.h"

#include "wx/intl.h"

#include "combinations/GOSetter.h"

#include "GODefinitionFile.h"
#include "GOManual.h"

GODivisionalButtonControl::GODivisionalButtonControl(
  GODefinitionFile *organfile,
  GOCombinationDefinition &divisionalTemplate,
  bool isSetter)
  : GOPushbuttonControl(organfile),
    m_divisional(organfile, divisionalTemplate, isSetter) {}

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
  m_divisional.Save(cfg);
}

void GODivisionalButtonControl::Push() {
  GOCombination::ExtraElementsSet elementSet;
  const GOCombination::ExtraElementsSet *pAddSet
    = m_organfile->GetSetter()->GetCrescendoAddSet(elementSet);
  GOManual *pManual = m_organfile->GetManual(m_divisional.GetManualNumber());

  m_divisional.Push(pAddSet);

  for (unsigned l = pManual->GetDivisionalCount(), k = 0; k < l; k++) {
    GODivisionalButtonControl *pDivisionalControl = pManual->GetDivisional(k);

    pDivisionalControl->Display(pDivisionalControl == this);
  }
}
