/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODrawstop.h"

#include <wx/intl.h>

#include "config/GOConfigEnum.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

#include "GOOrganModel.h"
#include "GOSwitch.h"

static const GOConfigEnum FUNCTION_TYPES({
  {wxT("Input"), GODrawstop::FUNCTION_INPUT},
  {wxT("And"), GODrawstop::FUNCTION_AND},
  {wxT("Or"), GODrawstop::FUNCTION_OR},
  {wxT("Not"), GODrawstop::FUNCTION_NOT},
  {wxT("Nand"), GODrawstop::FUNCTION_NAND},
  {wxT("Nor"), GODrawstop::FUNCTION_NOR},
  {wxT("Xor"), GODrawstop::FUNCTION_XOR},
});

GODrawstop::GODrawstop(
  GOOrganModel &organModel,
  const wxString &midiTypeCode,
  const wxString &midiTypeName)
  : GOButtonControl(
    organModel, midiTypeCode, midiTypeName, MIDI_RECV_DRAWSTOP, false),
    m_Type(FUNCTION_INPUT),
    m_GCState(0),
    m_ControlledDrawstops(),
    m_ControllingDrawstops(),
    m_IsToStoreInDivisional(false),
    m_IsToStoreInGeneral(false) {}

void GODrawstop::RegisterControlled(GODrawstop *sw) {
  m_ControlledDrawstops.push_back(sw);
}

void GODrawstop::UnRegisterControlled(GODrawstop *sw) {
  auto end = m_ControlledDrawstops.end();
  auto pos = std::find(m_ControlledDrawstops.begin(), end, sw);

  m_ControlledDrawstops.erase(pos);
}

void GODrawstop::ClearControllingDrawstops() {
  for (auto pControlling : m_ControllingDrawstops)
    pControlling->UnRegisterControlled(this);
  m_ControllingDrawstops.clear();
}

void GODrawstop::AddControllingDrawstop(
  GODrawstop *pDrawStop, unsigned switchN, const wxString &group) {
  auto end = m_ControllingDrawstops.end();

  if (m_Type == FUNCTION_INPUT)
    throw wxString::Format(_("Switch %d already assigned to %s"), group);
  if (std::find(m_ControllingDrawstops.begin(), end, pDrawStop) != end)
    throw wxString::Format(
      _("Switch %d already assigned to %s"), switchN, group);
  if (pDrawStop == this)
    throw wxString::Format(_("Drawstop %s can't reference to itself"), group);
  pDrawStop->RegisterControlled(this);
  m_ControllingDrawstops.push_back(pDrawStop);
}

void GODrawstop::SetFunctionType(GOFunctionType newFunctionType) {
  if (newFunctionType != m_Type) {
    if (m_Type == FUNCTION_INPUT)
      ClearControllingDrawstops();
    else if (m_Type == FUNCTION_NOT && m_ControllingDrawstops.size() > 1)
      throw wxString::Format(
        _("A NOT Switch must not have more than one controlling switches"));
    m_Type = newFunctionType;
  }
}

void GODrawstop::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  m_Type = FUNCTION_INPUT;
  m_Engaged = cfg.ReadBoolean(CMBSetting, group, wxT("DefaultToEngaged"));
  m_GCState = 0;
  m_IsToStoreInDivisional = true;
  m_IsToStoreInGeneral = true;
  GOButtonControl::Init(cfg, group, name);
}

void GODrawstop::SetupIsToStoreInCmb() {
  const bool isControlledByUser = !IsReadOnly();

  m_IsToStoreInDivisional
    = r_OrganModel.CombinationsStoreNonDisplayedDrawstops()
    || isControlledByUser;
  m_IsToStoreInGeneral = r_OrganModel.CombinationsStoreNonDisplayedDrawstops()
    || isControlledByUser;
}

void GODrawstop::Load(GOConfigReader &cfg, const wxString &group) {
  m_Type = (GOFunctionType)cfg.ReadEnum(
    ODFSetting, group, wxT("Function"), FUNCTION_TYPES, false, FUNCTION_INPUT);

  if (m_Type == FUNCTION_INPUT) {
    m_Engaged = cfg.ReadBoolean(ODFSetting, group, wxT("DefaultToEngaged"));
    m_Engaged = cfg.ReadBoolean(
      CMBSetting, group, wxT("DefaultToEngaged"), false, m_Engaged);
    m_GCState
      = cfg.ReadInteger(ODFSetting, group, wxT("GCState"), -1, 1, false, 0);
  } else {
    m_ReadOnly = true;
    unsigned cnt = 0;

    if (m_Type == FUNCTION_NOT)
      cnt = 1;
    else if (
      m_Type == FUNCTION_AND || m_Type == FUNCTION_OR || m_Type == FUNCTION_NAND
      || m_Type == FUNCTION_NOR || m_Type == FUNCTION_XOR) {
      cnt = cfg.ReadInteger(
        ODFSetting,
        group,
        wxT("SwitchCount"),
        1,
        r_OrganModel.GetSwitchCount(),
        true,
        1);
    }
    for (unsigned i = 0; i < cnt; i++) {
      unsigned no = cfg.ReadInteger(
        ODFSetting,
        group,
        wxString::Format(wxT("Switch%03d"), i + 1),
        1,
        r_OrganModel.GetSwitchCount(),
        true,
        1);

      AddControllingDrawstop(r_OrganModel.GetSwitch(no - 1), no, group);
    }
  }

  GOButtonControl::Load(cfg, group);
  SetupIsToStoreInCmb();
  m_IsToStoreInDivisional = cfg.ReadBoolean(
    ODFSetting,
    group,
    wxT("StoreInDivisional"),
    false,
    m_IsToStoreInDivisional);
  m_IsToStoreInGeneral = cfg.ReadBoolean(
    ODFSetting, group, wxT("StoreInGeneral"), false, m_IsToStoreInGeneral);
}

void GODrawstop::Save(GOConfigWriter &cfg) {
  if (!IsReadOnly())
    cfg.WriteBoolean(m_group, wxT("DefaultToEngaged"), IsEngaged());
  GOButtonControl::Save(cfg);
}

void GODrawstop::SetResultState(bool resState) {
  if (IsEngaged() != resState) {
    Display(resState);
    // must be before calling m_ControlledDrawstops[i]->Update();
    OnDrawstopStateChanged(resState);
    for (auto *pDrawstop : m_ControlledDrawstops)
      pDrawstop->Update(); // reads IsEngaged()
  }
}

void GODrawstop::Reset() {
  if (!IsReadOnly() && m_GCState >= 0) {
    if (m_GCState == 0) {
      // Clear all internal states
      for (auto &intState : m_InternalStates)
        intState.second = false;
      SetResultState(false);
    } else
      SetButtonState(true);
  }
}

bool GODrawstop::CalculateResultState(bool includeDefault) const {
  bool resState = false;

  for (const auto &intState : m_InternalStates)
    if (includeDefault || !intState.first.IsEmpty())
      resState = resState || intState.second;
  return resState;
}

void GODrawstop::SetInternalState(bool on, const wxString &stateName) {
  bool &internalState = m_InternalStates[stateName];

  if (internalState != on) {
    internalState = on;
    SetResultState(CalculateResultState(true));
  }
}

void GODrawstop::SetButtonState(bool on) {
  // we prohibit changing the button state while it is engaged by a crescendo
  if (!IsReadOnly() && !CalculateResultState(false))
    SetDrawStopState(on);
}

void GODrawstop::SetCombinationState(bool on, const wxString &stateName) {
  if (!IsReadOnly())
    SetInternalState(on, stateName);
}

void GODrawstop::StartPlayback() {
  GOButtonControl::StartPlayback();
  Update();
}

void GODrawstop::Update() {
  bool state;
  switch (m_Type) {
  case FUNCTION_INPUT:
    SetDrawStopState(IsEngaged());
    break;

  case FUNCTION_AND:
  case FUNCTION_NAND:
    state = true;
    for (unsigned i = 0; i < m_ControllingDrawstops.size(); i++)
      state = state && m_ControllingDrawstops[i]->IsEngaged();
    if (m_Type == FUNCTION_NAND)
      SetDrawStopState(!state);
    else
      SetDrawStopState(state);
    break;

  case FUNCTION_OR:
  case FUNCTION_NOR:
    state = false;
    for (unsigned i = 0; i < m_ControllingDrawstops.size(); i++)
      state = state || m_ControllingDrawstops[i]->IsEngaged();
    if (m_Type == FUNCTION_NOR)
      SetDrawStopState(!state);
    else
      SetDrawStopState(state);
    break;

  case FUNCTION_XOR:
    state = false;
    for (unsigned i = 0; i < m_ControllingDrawstops.size(); i++)
      state = state != m_ControllingDrawstops[i]->IsEngaged();
    SetDrawStopState(state);
    break;

  case FUNCTION_NOT:
    state = m_ControllingDrawstops[0]->IsEngaged();
    SetDrawStopState(!state);
    break;
  }
}
