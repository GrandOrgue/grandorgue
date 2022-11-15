/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGeneralCombination.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "combinations/GOSetter.h"
#include "combinations/control/GODivisionalButtonControl.h"
#include "combinations/control/GOGeneralButtonControl.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "model/GOManual.h"

#include "GOOrganController.h"

GOGeneralCombination::GOGeneralCombination(
  GOCombinationDefinition &general_template,
  GOOrganController *organController,
  bool is_setter)
  : GOCombination(general_template, organController),
    m_OrganController(organController),
    m_IsSetter(is_setter) {}

void GOGeneralCombination::Load(GOConfigReader &cfg, wxString group) {
  m_OrganController->RegisterSaveableObject(this);
  m_group = group;

  m_Protected
    = cfg.ReadBoolean(ODFSetting, group, wxT("Protected"), false, false);

  /* skip ODF settings */
  UpdateState();
  if (!m_IsSetter) {
    wxString buffer;
    int pos;
    std::vector<bool> used(m_State.size());
    unsigned NumberOfStops = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfStops"),
      0,
      m_OrganController->GetStopCount());
    unsigned NumberOfCouplers = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfCouplers"),
      0,
      m_OrganController->GetODFCouplerCount());
    unsigned NumberOfTremulants = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfTremulants"),
      0,
      m_OrganController->GetTremulantCount());
    unsigned NumberOfSwitches = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfSwitches"),
      0,
      m_OrganController->GetSwitchCount(),
      false,
      0);
    unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfDivisionalCouplers"),
      0,
      m_OrganController->GetDivisionalCouplerCount(),
      m_OrganController->GeneralsStoreDivisionalCouplers());

    for (unsigned i = 0; i < NumberOfStops; i++) {
      buffer.Printf(wxT("StopManual%03d"), i + 1);
      unsigned m = cfg.ReadInteger(
        ODFSetting,
        m_group,
        buffer,
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      buffer.Printf(wxT("StopNumber%03d"), i + 1);
      unsigned cnt = m_OrganController->GetManual(m)->GetStopCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_STOP, m, abs(s));
      if (pos >= 0) {
        if (used[pos]) {
          wxLogError(
            _("Duplicate combination entry %s in %s"),
            buffer.c_str(),
            m_group.c_str());
        }
        used[pos] = true;
      } else {
        wxLogError(
          _("Invalid combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
    }

    for (unsigned i = 0; i < NumberOfCouplers; i++) {
      buffer.Printf(wxT("CouplerManual%03d"), i + 1);
      unsigned m = cfg.ReadInteger(
        ODFSetting,
        m_group,
        buffer,
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      buffer.Printf(wxT("CouplerNumber%03d"), i + 1);
      unsigned cnt = m_OrganController->GetManual(m)->GetODFCouplerCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_COUPLER, m, abs(s));
      if (pos >= 0) {
        if (used[pos]) {
          wxLogError(
            _("Duplicate combination entry %s in %s"),
            buffer.c_str(),
            m_group.c_str());
        }
        used[pos] = true;
      } else {
        wxLogError(
          _("Invalid combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
    }

    for (unsigned i = 0; i < NumberOfTremulants; i++) {
      buffer.Printf(wxT("TremulantNumber%03d"), i + 1);
      unsigned cnt = m_OrganController->GetTremulantCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_TREMULANT, -1, abs(s));
      if (pos >= 0) {
        if (used[pos]) {
          wxLogError(
            _("Duplicate combination entry %s in %s"),
            buffer.c_str(),
            m_group.c_str());
        }
        used[pos] = true;
      } else {
        wxLogError(
          _("Invalid combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
    }

    for (unsigned i = 0; i < NumberOfSwitches; i++) {
      buffer.Printf(wxT("SwitchNumber%03d"), i + 1);
      unsigned cnt = m_OrganController->GetSwitchCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_SWITCH, -1, abs(s));
      if (pos >= 0) {
        if (used[pos]) {
          wxLogError(
            _("Duplicate combination entry %s in %s"),
            buffer.c_str(),
            m_group.c_str());
        }
        used[pos] = true;
      } else {
        wxLogError(
          _("Invalid combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
    }

    for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++) {
      buffer.Printf(wxT("DivisionalCouplerNumber%03d"), i + 1);
      unsigned cnt = m_OrganController->GetDivisionalCouplerCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER, -1, abs(s));
      if (pos >= 0) {
        if (used[pos]) {
          wxLogError(
            _("Duplicate combination entry %s in %s"),
            buffer.c_str(),
            m_group.c_str());
        }
        used[pos] = true;
      } else {
        wxLogError(
          _("Invalid combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
    }
  }
}

void GOGeneralCombination::LoadCombination(GOConfigReader &cfg) {
  GOSettingType type = CMBSetting;
  if (!m_IsSetter)
    if (
      cfg.ReadInteger(
        CMBSetting,
        m_group,
        wxT("NumberOfStops"),
        -1,
        m_OrganController->GetStopCount(),
        false,
        -1)
      == -1)
      type = ODFSetting;
  wxString buffer;
  unsigned NumberOfStops = cfg.ReadInteger(
    type, m_group, wxT("NumberOfStops"), 0, m_OrganController->GetStopCount());
  unsigned NumberOfCouplers = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfCouplers"),
    0,
    type == CMBSetting ? m_OrganController->GetCouplerCount()
                       : m_OrganController->GetODFCouplerCount());
  unsigned NumberOfTremulants = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    m_OrganController->GetTremulantCount());
  unsigned NumberOfSwitches = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    m_OrganController->GetSwitchCount(),
    false,
    0);
  unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfDivisionalCouplers"),
    0,
    m_OrganController->GetDivisionalCouplerCount(),
    m_OrganController->GeneralsStoreDivisionalCouplers());

  int pos;
  UpdateState();
  for (unsigned i = 0; i < m_State.size(); i++)
    m_State[i] = -1;

  for (unsigned i = 0; i < NumberOfStops; i++) {
    buffer.Printf(wxT("StopManual%03d"), i + 1);
    unsigned m = cfg.ReadInteger(
      type,
      m_group,
      buffer,
      m_OrganController->GetFirstManualIndex(),
      m_OrganController->GetManualAndPedalCount());
    buffer.Printf(wxT("StopNumber%03d"), i + 1);
    /*
    unsigned cnt = m_OrganController->GetManual(m)->GetStopCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
     */
    int s = cfg.ReadInteger(type, m_group, buffer, -999, 999);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_STOP, m, abs(s));
    if (pos >= 0) {
      if (m_State[pos] != -1) {
        wxLogError(
          _("Duplicate combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
      m_State[pos] = (s > 0) ? 1 : 0;
    } else {
      wxLogError(
        _("Invalid combination entry %s in %s"),
        buffer.c_str(),
        m_group.c_str());
    }
  }

  for (unsigned i = 0; i < NumberOfCouplers; i++) {
    buffer.Printf(wxT("CouplerManual%03d"), i + 1);
    unsigned m = cfg.ReadInteger(
      type,
      m_group,
      buffer,
      m_OrganController->GetFirstManualIndex(),
      m_OrganController->GetManualAndPedalCount());
    buffer.Printf(wxT("CouplerNumber%03d"), i + 1);
    unsigned cnt = type == CMBSetting
      ? m_OrganController->GetManual(m)->GetCouplerCount()
      : m_OrganController->GetManual(m)->GetODFCouplerCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_COUPLER, m, abs(s));
    if (pos >= 0) {
      if (m_State[pos] != -1) {
        wxLogError(
          _("Duplicate combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
      m_State[pos] = (s > 0) ? 1 : 0;
    } else {
      wxLogError(
        _("Invalid combination entry %s in %s"),
        buffer.c_str(),
        m_group.c_str());
    }
  }

  for (unsigned i = 0; i < NumberOfTremulants; i++) {
    buffer.Printf(wxT("TremulantNumber%03d"), i + 1);
    unsigned cnt = m_OrganController->GetTremulantCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_TREMULANT, -1, abs(s));
    if (pos >= 0) {
      if (m_State[pos] != -1) {
        wxLogError(
          _("Duplicate combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
      m_State[pos] = (s > 0) ? 1 : 0;
    } else {
      wxLogError(
        _("Invalid combination entry %s in %s"),
        buffer.c_str(),
        m_group.c_str());
    }
  }

  for (unsigned i = 0; i < NumberOfSwitches; i++) {
    buffer.Printf(wxT("SwitchNumber%03d"), i + 1);
    unsigned cnt = m_OrganController->GetSwitchCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_SWITCH, -1, abs(s));
    if (pos >= 0) {
      if (m_State[pos] != -1) {
        wxLogError(
          _("Duplicate combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
      m_State[pos] = (s > 0) ? 1 : 0;
    } else {
      wxLogError(
        _("Invalid combination entry %s in %s"),
        buffer.c_str(),
        m_group.c_str());
    }
  }

  for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++) {
    buffer.Printf(wxT("DivisionalCouplerNumber%03d"), i + 1);
    unsigned cnt = m_OrganController->GetDivisionalCouplerCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER, -1, abs(s));
    if (pos >= 0) {
      if (m_State[pos] != -1) {
        wxLogError(
          _("Duplicate combination entry %s in %s"),
          buffer.c_str(),
          m_group.c_str());
      }
      m_State[pos] = (s > 0) ? 1 : 0;
    } else {
      wxLogError(
        _("Invalid combination entry %s in %s"),
        buffer.c_str(),
        m_group.c_str());
    }
  }
}

void GOGeneralCombination::Push(
  ExtraElementsSet const *extraSet, bool isFromCrescendo) {
  GOCombination::PushLocal(extraSet);

  if (!isFromCrescendo || !extraSet) { // Crescendo in add mode: not to switch
                                       // off combination
                                       // buttons
    for (unsigned k = 0; k < m_OrganController->GetGeneralCount(); k++) {
      GOGeneralButtonControl *general = m_OrganController->GetGeneral(k);
      general->Display(&general->GetGeneral() == this);
    }

    for (unsigned j = m_OrganController->GetFirstManualIndex();
         j <= m_OrganController->GetManualAndPedalCount();
         j++) {
      for (unsigned k = 0;
           k < m_OrganController->GetManual(j)->GetDivisionalCount();
           k++)
        m_OrganController->GetManual(j)->GetDivisional(k)->Display(false);
    }
  }

  m_OrganController->GetSetter()->ResetDisplay();
}

void GOGeneralCombination::Save(GOConfigWriter &cfg) {
  UpdateState();
  const std::vector<GOCombinationDefinition::CombinationSlot> &elements
    = m_Template.GetCombinationElements();

  wxString buffer;
  unsigned stop_count = 0;
  unsigned coupler_count = 0;
  unsigned tremulant_count = 0;
  unsigned switch_count = 0;
  unsigned divisional_coupler_count = 0;

  for (unsigned i = 0; i < elements.size(); i++) {
    if (m_State[i] == -1)
      continue;
    int value = m_State[i] == 1 ? elements[i].index : -elements[i].index;
    switch (elements[i].type) {
    case GOCombinationDefinition::COMBINATION_STOP:
      stop_count++;
      buffer.Printf(wxT("StopManual%03d"), stop_count);
      cfg.WriteInteger(m_group, buffer, elements[i].manual);
      buffer.Printf(wxT("StopNumber%03d"), stop_count);
      cfg.WriteInteger(m_group, buffer, value);
      break;

    case GOCombinationDefinition::COMBINATION_COUPLER:
      coupler_count++;
      buffer.Printf(wxT("CouplerManual%03d"), coupler_count);
      cfg.WriteInteger(m_group, buffer, elements[i].manual);
      buffer.Printf(wxT("CouplerNumber%03d"), coupler_count);
      cfg.WriteInteger(m_group, buffer, value);
      break;

    case GOCombinationDefinition::COMBINATION_TREMULANT:
      tremulant_count++;
      buffer.Printf(wxT("TremulantNumber%03d"), tremulant_count);
      cfg.WriteInteger(m_group, buffer, value);
      break;

    case GOCombinationDefinition::COMBINATION_SWITCH:
      switch_count++;
      buffer.Printf(wxT("SwitchNumber%03d"), switch_count);
      cfg.WriteInteger(m_group, buffer, value);
      break;

    case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
      divisional_coupler_count++;
      buffer.Printf(
        wxT("DivisionalCouplerNumber%03d"), divisional_coupler_count);
      cfg.WriteInteger(m_group, buffer, value);
      break;
    }
  }

  cfg.WriteInteger(m_group, wxT("NumberOfStops"), stop_count);
  cfg.WriteInteger(m_group, wxT("NumberOfCouplers"), coupler_count);
  cfg.WriteInteger(m_group, wxT("NumberOfTremulants"), tremulant_count);
  cfg.WriteInteger(m_group, wxT("NumberOfSwitches"), switch_count);
  cfg.WriteInteger(
    m_group, wxT("NumberOfDivisionalCouplers"), divisional_coupler_count);
}
