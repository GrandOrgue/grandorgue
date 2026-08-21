/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
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

// The (defaulted) constructor and destructor can't be left implicit in the
// header: GOCallbackButtonControl is only forward-declared there, and any
// other translation unit implicitly constructing/destroying a
// GOVirtualCouplerController (e.g. GOOrganController.cpp, which holds one as
// a by-value member) would need std::unique_ptr<GOCallbackButtonControl>'s
// constructor/deleter instantiated with an incomplete type. Declaring them
// here and defining them in this .cpp, where the type is complete, keeps
// that instantiation confined to this file.
GOVirtualCouplerController::GOVirtualCouplerController() = default;
GOVirtualCouplerController::~GOVirtualCouplerController() = default;

static GOVirtualCouplerController::CouplerSetKey make_key(
  unsigned srcManualN, unsigned dstManualN) {
  return std::make_pair(srcManualN, dstManualN);
}

static void load_coupler(
  GOOrganModel &organModel,
  GOConfigReader &cfg,
  unsigned srcManualN,
  unsigned dstManualN,
  GOVirtualCouplerController::CouplerSet &couplerSet,
  bool unisonOff,
  GOCoupler::GOCouplerType couplerType,
  int keyshift,
  const wxString &cfgGroupFmt,
  const wxString &recorderNameFmt,
  const wxString &couplerLabel) {
  GOManual *pSrcManual = organModel.GetManual(srcManualN);
  GOCoupler *pCoupler = new GOCoupler(
    organModel, srcManualN, true, couplerSet.m_CouplersContext.get());

  pCoupler->Init(
    cfg,
    wxString::Format(cfgGroupFmt, srcManualN, dstManualN),
    couplerLabel,
    unisonOff,
    false,
    keyshift,
    dstManualN,
    couplerType);
  pCoupler->SetElementId(organModel.GetRecorderElementID(
    wxString::Format(recorderNameFmt, srcManualN, dstManualN)));
  pSrcManual->AddCoupler(pCoupler);
  couplerSet.m_CouplerPtrs.push_back(pCoupler);
}

static wxString WX_03U = wxT("%03u");

void GOVirtualCouplerController::Init(
  GOOrganModel &organModel, GOConfigReader &cfg) {
  for (unsigned srcManualN = organModel.GetFirstManualIndex();
       srcManualN <= organModel.GetManualAndPedalCount();
       srcManualN++) {
    const GOMidiObjectContext *pSrcVirtCouplerContext
      = organModel.GetManual(srcManualN)->GetVirtualCouplersContext();

    for (unsigned int dstManualN = organModel.GetFirstManualIndex();
         dstManualN < organModel.GetODFManualCount();
         dstManualN++) {
      CouplerSet &couplers = m_CouplerSets[make_key(srcManualN, dstManualN)];

      couplers.m_CouplersContext
        = std::unique_ptr<GOMidiObjectContext>(new GOMidiObjectContext(
          wxString::Format(WX_03U, dstManualN),
          organModel.GetManual(dstManualN)->GetNameForContext(),
          pSrcVirtCouplerContext));

      load_coupler(
        organModel,
        cfg,
        srcManualN,
        dstManualN,
        couplers,
        false,
        GOCoupler::COUPLER_NORMAL,
        -12,
        wxT("SetterManual%03dCoupler%03dT16"),
        wxT("S%dM%dC16"),
        _("16"));

      load_coupler(
        organModel,
        cfg,
        srcManualN,
        dstManualN,
        couplers,
        srcManualN == dstManualN,
        GOCoupler::COUPLER_NORMAL,
        0,
        wxT("SetterManual%03dCoupler%03dT8"),
        wxT("S%dM%dC8"),
        srcManualN != dstManualN ? _("8") : _("U.O."));

      load_coupler(
        organModel,
        cfg,
        srcManualN,
        dstManualN,
        couplers,
        false,
        GOCoupler::COUPLER_NORMAL,
        12,
        wxT("SetterManual%03dCoupler%03dT4"),
        wxT("S%dM%dC4"),
        _("4"));

      load_coupler(
        organModel,
        cfg,
        srcManualN,
        dstManualN,
        couplers,
        false,
        GOCoupler::COUPLER_BASS,
        0,
        wxT("SetterManual%03dCoupler%03dBAS"),
        wxT("S%dM%dCB"),
        _("BAS"));

      load_coupler(
        organModel,
        cfg,
        srcManualN,
        dstManualN,
        couplers,
        false,
        GOCoupler::COUPLER_MELODY,
        0,
        wxT("SetterManual%03dCoupler%03dMEL"),
        wxT("S%dM%dCM"),
        _("MEL"));

      // Not std::make_unique: it would convert `this` to GOButtonCallback*
      // inside library code, outside this class's own scope, which fails
      // since GOButtonCallback is a private base here.
      couplers.m_ButtonCoupleThrough = std::unique_ptr<GOCallbackButtonControl>(
        new GOCallbackButtonControl(organModel, this, false, false));
      couplers.m_ButtonCoupleThrough->SetContext(
        couplers.m_CouplersContext.get());
      couplers.m_ButtonCoupleThrough->Init(
        cfg,
        wxString::Format(
          wxT("SetterManual%03dCoupler%03dThrough"), srcManualN, dstManualN),
        _("Couple Through"));
    }
  }
}

static wxString WX_COUPLE_THROUGH = wxT("CoupleThrough");

void GOVirtualCouplerController::Load(
  GOOrganModel &organModel, GOConfigReader &cfg) {
  Init(organModel, cfg);
  for (auto &e : m_CouplerSets) {
    GOCallbackButtonControl *pCoupleThrough
      = e.second.m_ButtonCoupleThrough.get();
    bool isCoupleThrough = cfg.ReadBoolean(
      CMBSetting, pCoupleThrough->GetGroup(), WX_COUPLE_THROUGH, false, false);

    pCoupleThrough->SetButtonState(isCoupleThrough);
  }
}

void GOVirtualCouplerController::Save(GOConfigWriter &cfg) {
  for (auto &e : m_CouplerSets) {
    GOCallbackButtonControl *pCoupleThrough
      = e.second.m_ButtonCoupleThrough.get();

    cfg.WriteBoolean(
      pCoupleThrough->GetGroup(),
      WX_COUPLE_THROUGH,
      pCoupleThrough->IsEngaged());
  }
}

// Can't be inline in the header: m_CouplerSets.clear() destroys
// m_ButtonCoupleThrough, needing the complete GOCallbackButtonControl type.
void GOVirtualCouplerController::Cleanup() { m_CouplerSets.clear(); }

GOCoupler *GOVirtualCouplerController::GetCoupler(
  unsigned fromManual, unsigned toManual, CouplerType type) const {
  const auto iter = m_CouplerSets.find(make_key(fromManual, toManual));

  return iter != m_CouplerSets.end() ? iter->second.m_CouplerPtrs[type]
                                     : nullptr;
}

GOButtonControl *GOVirtualCouplerController::GetCouplerThrough(
  unsigned fromManual, unsigned toManual) const {
  const auto iter = m_CouplerSets.find(make_key(fromManual, toManual));

  return iter != m_CouplerSets.end() ? iter->second.m_ButtonCoupleThrough.get()
                                     : nullptr;
}

void GOVirtualCouplerController::ButtonStateChanged(
  GOButtonControl *button, bool newState) {
  for (auto &e : m_CouplerSets)
    if (e.second.m_ButtonCoupleThrough.get() == button)
      for (auto pCoupler : e.second.m_CouplerPtrs) {
        pCoupler->SetRecursive(newState);
        pCoupler->RefreshState();
      }
}
