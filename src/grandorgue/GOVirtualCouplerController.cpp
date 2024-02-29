/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOVirtualCouplerController.h"

#include <wx/intl.h>

#include "config/GOConfigWriter.h"
#include "control/GOCallbackButtonControl.h"
#include "model/GOCoupler.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"

void load_coupler(
  GOOrganModel &organModel,
  GOConfigReader &cfg,
  GOVirtualCouplerController::ManualCouplerSet &manualCouplers,
  unsigned srcManualN,
  unsigned dstManualN,
  bool unisonOff,
  GOCoupler::GOCouplerType couplerType,
  int keyshift,
  const wxString &cfgGroupFmt,
  const wxString &recorderNameFmt,
  const wxString &couplerLabel) {
  GOManual *pSrcManual = organModel.GetManual(srcManualN);
  GOCoupler *pCoupler = new GOCoupler(organModel, srcManualN);

  pCoupler->Init(
    cfg,
    wxString::Format(cfgGroupFmt, srcManualN, dstManualN),
    couplerLabel,
    unisonOff,
    false,
    keyshift,
    dstManualN,
    couplerType);
  pCoupler->SetElementID(organModel.GetRecorderElementID(
    wxString::Format(recorderNameFmt, srcManualN, dstManualN)));
  pSrcManual->AddCoupler(pCoupler);
  manualCouplers.push_back(pCoupler);
}

GOVirtualCouplerController::CouplerSetKey make_key(
  unsigned srcManualN, unsigned dstManualN) {
  return std::make_pair(srcManualN, dstManualN);
}

void GOVirtualCouplerController::Init(
  GOOrganModel &organModel, GOConfigReader &cfg) {
  for (unsigned srcManualN = organModel.GetFirstManualIndex();
       srcManualN <= organModel.GetManualAndPedalCount();
       srcManualN++) {
    for (unsigned int dstManualN = organModel.GetFirstManualIndex();
         dstManualN < organModel.GetODFManualCount();
         dstManualN++) {
      CouplerSetKey couplerSetKey = make_key(srcManualN, dstManualN);
      ManualCouplerSet &manualCouplers = m_CouplerPtrs[couplerSetKey];

      load_coupler(
        organModel,
        cfg,
        manualCouplers,
        srcManualN,
        dstManualN,
        false,
        GOCoupler::COUPLER_NORMAL,
        -12,
        wxT("SetterManual%03dCoupler%03dT16"),
        wxT("S%dM%dC16"),
        _("16"));

      load_coupler(
        organModel,
        cfg,
        manualCouplers,
        srcManualN,
        dstManualN,
        srcManualN == dstManualN,
        GOCoupler::COUPLER_NORMAL,
        0,
        wxT("SetterManual%03dCoupler%03dT8"),
        wxT("S%dM%dC8"),
        srcManualN != dstManualN ? _("8") : _("U.O."));

      load_coupler(
        organModel,
        cfg,
        manualCouplers,
        srcManualN,
        dstManualN,
        false,
        GOCoupler::COUPLER_NORMAL,
        12,
        wxT("SetterManual%03dCoupler%03dT4"),
        wxT("S%dM%dC4"),
        _("4"));

      load_coupler(
        organModel,
        cfg,
        manualCouplers,
        srcManualN,
        dstManualN,
        false,
        GOCoupler::COUPLER_BASS,
        0,
        wxT("SetterManual%03dCoupler%03dBAS"),
        wxT("S%dM%dCB"),
        _("BAS"));

      load_coupler(
        organModel,
        cfg,
        manualCouplers,
        srcManualN,
        dstManualN,
        false,
        GOCoupler::COUPLER_MELODY,
        0,
        wxT("SetterManual%03dCoupler%03dMEL"),
        wxT("S%dM%dCM"),
        _("MEL"));

      GOCallbackButtonControl *pCoupleThrough
        = new GOCallbackButtonControl(organModel, this, false, false);

      pCoupleThrough->Init(
        cfg,
        wxString::Format(
          wxT("SetterManual%03dCoupler%03dThrough"), srcManualN, dstManualN),
        _("Couple Through"));
      m_CoupleThroughPtrs[couplerSetKey] = pCoupleThrough;
    }
  }
}

static wxString WX_COUPLE_THROUGH = wxT("CoupleThrough");

void GOVirtualCouplerController::Load(
  GOOrganModel &organModel, GOConfigReader &cfg) {
  Init(organModel, cfg);
  for (auto e : m_CoupleThroughPtrs) {
    GOCallbackButtonControl *pCoupleThrough = e.second;
    bool isCoupleThrough = cfg.ReadBoolean(
      CMBSetting, pCoupleThrough->GetGroup(), WX_COUPLE_THROUGH, false, false);

    pCoupleThrough->Set(isCoupleThrough);
  }
}

void GOVirtualCouplerController::Save(GOConfigWriter &cfg) {
  for (auto e : m_CoupleThroughPtrs) {
    GOCallbackButtonControl *pCoupleThrough = e.second;

    cfg.WriteBoolean(
      pCoupleThrough->GetGroup(),
      WX_COUPLE_THROUGH,
      pCoupleThrough->IsEngaged());
  }
}

GOCoupler *GOVirtualCouplerController::GetCoupler(
  unsigned fromManual, unsigned toManual, CouplerType type) const {
  const auto iter = m_CouplerPtrs.find(make_key(fromManual, toManual));

  return iter != m_CouplerPtrs.end() ? iter->second[type] : nullptr;
}

GOButtonControl *GOVirtualCouplerController::GetCouplerThrough(
  unsigned fromManual, unsigned toManual) const {
  const auto iter = m_CoupleThroughPtrs.find(make_key(fromManual, toManual));

  return iter != m_CoupleThroughPtrs.end() ? iter->second : nullptr;
}

void GOVirtualCouplerController::ButtonStateChanged(
  GOButtonControl *button, bool newState) {
  for (auto e : m_CoupleThroughPtrs)
    if (e.second == button) {
      const CouplerSetKey &couplerSetKey = e.first;
      ManualCouplerSet &manualCouplers = m_CouplerPtrs[couplerSetKey];

      for (auto pCoupler : manualCouplers) {
        pCoupler->SetRecursive(newState);
        pCoupler->RefreshState();
      }
    }
}
