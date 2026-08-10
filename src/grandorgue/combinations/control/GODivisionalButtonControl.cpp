/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalButtonControl.h"

#include "wx/intl.h"

#include "model/GOManual.h"
#include "model/GOOrganModel.h"

GODivisionalButtonControl::GODivisionalButtonControl(
  GOOrganModel &organModel,
  unsigned manualIndex,
  unsigned divisionalIndex,
  const GOMidiObjectContext *pContext)
  : GOPushbuttonControl(organModel, OBJECT_TYPE_DIVISIONAL),
    r_OrganModel(organModel),
    m_ManualIndex(manualIndex),
    m_DivisionalIndex(divisionalIndex),
    m_combination(organModel, manualIndex, false) {
  SetContext(pContext);
}

void GODivisionalButtonControl::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  GOPushbuttonControl::Init(cfg, group, name);
  m_combination.Init(group, m_DivisionalIndex);
}

void GODivisionalButtonControl::Load(
  GOConfigReader &cfg, const wxString &group) {
  GOPushbuttonControl::Load(cfg, group);
  m_combination.Load(cfg, group, m_DivisionalIndex);
}

void GODivisionalButtonControl::LoadCombination(GOConfigReader &cfg) {
  m_combination.LoadCombination(cfg);
}

void GODivisionalButtonControl::Save(GOConfigWriter &cfg) {
  GOPushbuttonControl::Save(cfg);
  m_combination.Save(cfg);
}

void GODivisionalButtonControl::Push() {
  r_OrganModel.ProcessPushDivisional(
    m_ManualIndex, [&](unsigned coupledManualIndex) {
      GOManual *pManual = r_OrganModel.GetManual(coupledManualIndex);
      GODivisionalButtonControl *pCoupledButton
        = (m_DivisionalIndex < pManual->GetDivisionalCount())
        ? pManual->GetDivisional(m_DivisionalIndex)
        : nullptr;
      return std::pair(
        pCoupledButton,
        pCoupledButton ? &pCoupledButton->m_combination : nullptr);
    });
}
