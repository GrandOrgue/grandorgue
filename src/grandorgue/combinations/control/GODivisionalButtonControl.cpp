/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalButtonControl.h"

#include "wx/intl.h"

#include "model/GOManual.h"
#include "model/GOOrganModel.h"

GODivisionalButtonControl::GODivisionalButtonControl(
  GOOrganModel &organModel,
  unsigned manualNumber,
  unsigned divisionalIndex,
  const GOMidiObjectContext *pContext)
  : GOPushbuttonControl(organModel, OBJECT_TYPE_DIVISIONAL),
    r_OrganModel(organModel),
    m_ManualN(manualNumber),
    m_DivisionalIndex(divisionalIndex),
    m_combination(organModel, manualNumber, false) {
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
  for (unsigned coupledManualN :
       r_OrganModel.GetCoupledManualsForDivisional(m_ManualN)) {
    GOManual *pManual = r_OrganModel.GetManual(coupledManualN);

    if (m_DivisionalIndex < pManual->GetDivisionalCount()) {
      GODivisionalButtonControl *pCoupledButton
        = pManual->GetDivisional(m_DivisionalIndex);

      r_OrganModel.PushDivisional(
        pCoupledButton->GetCombination(),
        m_ManualN,
        coupledManualN,
        pCoupledButton);
    }
  }
}
