/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODrawStop.h"

#include <wx/intl.h>

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

#include "GOOrganModel.h"
#include "GOSwitch.h"

const struct IniFileEnumEntry GODrawstop::m_function_types[] = {
  {wxT("Input"), FUNCTION_INPUT},
  {wxT("And"), FUNCTION_AND},
  {wxT("Or"), FUNCTION_OR},
  {wxT("Not"), FUNCTION_NOT},
  {wxT("Nand"), FUNCTION_NAND},
  {wxT("Nor"), FUNCTION_NOR},
  {wxT("Xor"), FUNCTION_XOR},
};

GODrawstop::GODrawstop(GOOrganModel &organModel)
  : GOButtonControl(organModel, MIDI_RECV_DRAWSTOP, false),
    m_Type(FUNCTION_INPUT),
    m_GCState(0),
    m_ActiveState(false),
    m_CombinationState(false),
    m_ControlledDrawstops(),
    m_ControllingDrawstops(),
    m_IsToStoreInDivisional(false),
    m_IsToStoreInGeneral(false) {}

void GODrawstop::RegisterControlled(GODrawstop *sw) {
  m_ControlledDrawstops.push_back(sw);
}

void GODrawstop::Init(GOConfigReader &cfg, wxString group, wxString name) {
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

void GODrawstop::Load(GOConfigReader &cfg, wxString group) {
  m_Type = (GOFunctionType)cfg.ReadEnum(
    ODFSetting,
    group,
    wxT("Function"),
    m_function_types,
    sizeof(m_function_types) / sizeof(m_function_types[0]),
    false,
    FUNCTION_INPUT);

  if (m_Type == FUNCTION_INPUT) {
    m_Engaged = cfg.ReadBoolean(ODFSetting, group, wxT("DefaultToEngaged"));
    m_Engaged = cfg.ReadBoolean(
      CMBSetting, group, wxT("DefaultToEngaged"), false, m_Engaged);
    m_GCState
      = cfg.ReadInteger(ODFSetting, group, wxT("GCState"), -1, 1, false, 0);
  } else {
    m_ReadOnly = true;
    unsigned cnt = 0;
    bool unique = true;
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
      GODrawstop *s = r_OrganModel.GetSwitch(no - 1);
      for (unsigned j = 0; j < m_ControllingDrawstops.size(); j++)
        if (unique && m_ControllingDrawstops[j] == s)
          throw wxString::Format(
            _("Switch %d already assigned to %s"), no, group.c_str());
      if (s == (GODrawstop *)this)
        throw wxString::Format(
          _("Drawstop %s can't reference to itself"), group.c_str());
      s->RegisterControlled(this);
      m_ControllingDrawstops.push_back(s);
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

void GODrawstop::Set(bool on) {
  if (IsEngaged() == on)
    return;
  Display(on);
  SetState(on);
}

void GODrawstop::Reset() {
  if (IsReadOnly())
    return;
  if (m_GCState < 0)
    return;
  Set(m_GCState > 0 ? true : false);
}

void GODrawstop::SetState(bool on) {
  if (IsActive() == on)
    return;
  if (IsReadOnly()) {
    Display(on);
  }
  m_ActiveState = on;
  ChangeState(on);
  for (unsigned i = 0; i < m_ControlledDrawstops.size(); i++)
    m_ControlledDrawstops[i]->Update();
}

void GODrawstop::SetCombination(bool on) {
  if (IsReadOnly())
    return;
  m_CombinationState = on;
  Set(on);
}

void GODrawstop::StartPlayback() {
  GOButtonControl::StartPlayback();
  Update();
}

void GODrawstop::Update() {
  bool state;
  switch (m_Type) {
  case FUNCTION_INPUT:
    SetState(IsEngaged());
    break;

  case FUNCTION_AND:
  case FUNCTION_NAND:
    state = true;
    for (unsigned i = 0; i < m_ControllingDrawstops.size(); i++)
      state = state && m_ControllingDrawstops[i]->IsActive();
    if (m_Type == FUNCTION_NAND)
      SetState(!state);
    else
      SetState(state);
    break;

  case FUNCTION_OR:
  case FUNCTION_NOR:
    state = false;
    for (unsigned i = 0; i < m_ControllingDrawstops.size(); i++)
      state = state || m_ControllingDrawstops[i]->IsActive();
    if (m_Type == FUNCTION_NOR)
      SetState(!state);
    else
      SetState(state);
    break;

  case FUNCTION_XOR:
    state = false;
    for (unsigned i = 0; i < m_ControllingDrawstops.size(); i++)
      state = state != m_ControllingDrawstops[i]->IsActive();
    SetState(state);
    break;

  case FUNCTION_NOT:
    state = m_ControllingDrawstops[0]->IsActive();
    SetState(!state);
    break;
  }
}
