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
  GOOrganModel &organModel, unsigned manualNumber, bool isSetter)
  : GOPushbuttonControl(organModel),
    r_OrganModel(organModel),
    m_combination(organModel, manualNumber, isSetter) {}

wxString GODivisionalButtonControl::GetMidiType() { return _("Divisional"); };

void GODivisionalButtonControl::Init(
  GOConfigReader &cfg,
  const wxString &group,
  int divisionalNumber,
  const wxString &name) {
  GOPushbuttonControl::Init(cfg, group, name);
  m_combination.Init(group, divisionalNumber);
}
void GODivisionalButtonControl::Load(
  GOConfigReader &cfg, const wxString &group, int divisionalNumber) {
  GOPushbuttonControl::Load(cfg, group);
  m_combination.Load(cfg, group, divisionalNumber);
}

void GODivisionalButtonControl::LoadCombination(GOConfigReader &cfg) {
  m_combination.LoadCombination(cfg);
}

void GODivisionalButtonControl::Save(GOConfigWriter &cfg) {
  GOPushbuttonControl::Save(cfg);
  m_combination.Save(cfg);
}

void GODivisionalButtonControl::Push() {
  organModel.PushDivisional(m_combination, this);
}
