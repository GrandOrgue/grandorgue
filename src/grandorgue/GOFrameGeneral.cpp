/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOFrameGeneral.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GODefinitionFile.h"
#include "GODivisional.h"
#include "GOGeneral.h"
#include "GOManual.h"
#include "GOSetter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOFrameGeneral::GOFrameGeneral(
  GOCombinationDefinition &general_template,
  GODefinitionFile *organfile,
  bool is_setter)
  : GOCombination(general_template, organfile),
    m_organfile(organfile),
    m_IsSetter(is_setter) {}

void GOFrameGeneral::Load(GOConfigReader &cfg, wxString group) {
  m_organfile->RegisterSaveableObject(this);
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
      m_organfile->GetStopCount());
    unsigned NumberOfCouplers = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfCouplers"),
      0,
      m_organfile->GetODFCouplerCount());
    unsigned NumberOfTremulants = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfTremulants"),
      0,
      m_organfile->GetTremulantCount());
    unsigned NumberOfSwitches = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfSwitches"),
      0,
      m_organfile->GetSwitchCount(),
      false,
      0);
    unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfDivisionalCouplers"),
      0,
      m_organfile->GetDivisionalCouplerCount(),
      m_organfile->GeneralsStoreDivisionalCouplers());

    for (unsigned i = 0; i < NumberOfStops; i++) {
      buffer.Printf(wxT("StopManual%03d"), i + 1);
      unsigned m = cfg.ReadInteger(
        ODFSetting,
        m_group,
        buffer,
        m_organfile->GetFirstManualIndex(),
        m_organfile->GetManualAndPedalCount());
      buffer.Printf(wxT("StopNumber%03d"), i + 1);
      unsigned cnt = m_organfile->GetManual(m)->GetStopCount();
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
        m_organfile->GetFirstManualIndex(),
        m_organfile->GetManualAndPedalCount());
      buffer.Printf(wxT("CouplerNumber%03d"), i + 1);
      unsigned cnt = m_organfile->GetManual(m)->GetODFCouplerCount();
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
      unsigned cnt = m_organfile->GetTremulantCount();
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
      unsigned cnt = m_organfile->GetSwitchCount();
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
      unsigned cnt = m_organfile->GetDivisionalCouplerCount();
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

void GOFrameGeneral::LoadCombination(GOConfigReader &cfg) {
  GOSettingType type = CMBSetting;
  if (!m_IsSetter)
    if (
      cfg.ReadInteger(
        CMBSetting,
        m_group,
        wxT("NumberOfStops"),
        -1,
        m_organfile->GetStopCount(),
        false,
        -1)
      == -1)
      type = ODFSetting;
  wxString buffer;
  unsigned NumberOfStops = cfg.ReadInteger(
    type, m_group, wxT("NumberOfStops"), 0, m_organfile->GetStopCount());
  unsigned NumberOfCouplers = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfCouplers"),
    0,
    type == CMBSetting ? m_organfile->GetCouplerCount()
                       : m_organfile->GetODFCouplerCount());
  unsigned NumberOfTremulants = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    m_organfile->GetTremulantCount());
  unsigned NumberOfSwitches = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    m_organfile->GetSwitchCount(),
    false,
    0);
  unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfDivisionalCouplers"),
    0,
    m_organfile->GetDivisionalCouplerCount(),
    m_organfile->GeneralsStoreDivisionalCouplers());

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
      m_organfile->GetFirstManualIndex(),
      m_organfile->GetManualAndPedalCount());
    buffer.Printf(wxT("StopNumber%03d"), i + 1);
    /*
    unsigned cnt = m_organfile->GetManual(m)->GetStopCount();
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
      m_organfile->GetFirstManualIndex(),
      m_organfile->GetManualAndPedalCount());
    buffer.Printf(wxT("CouplerNumber%03d"), i + 1);
    unsigned cnt = type == CMBSetting
      ? m_organfile->GetManual(m)->GetCouplerCount()
      : m_organfile->GetManual(m)->GetODFCouplerCount();
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
    unsigned cnt = m_organfile->GetTremulantCount();
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
    unsigned cnt = m_organfile->GetSwitchCount();
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
    unsigned cnt = m_organfile->GetDivisionalCouplerCount();
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

void GOFrameGeneral::Push() {
  bool used = GOCombination::PushLocal();

  for (unsigned k = 0; k < m_organfile->GetGeneralCount(); k++) {
    GOGeneral *general = m_organfile->GetGeneral(k);
    general->Display(&general->GetGeneral() == this && used);
  }

  for (unsigned j = m_organfile->GetFirstManualIndex();
       j <= m_organfile->GetManualAndPedalCount();
       j++) {
    for (unsigned k = 0; k < m_organfile->GetManual(j)->GetDivisionalCount();
         k++)
      m_organfile->GetManual(j)->GetDivisional(k)->Display(false);
  }

  m_organfile->GetSetter()->ResetDisplay();
}

void GOFrameGeneral::Save(GOConfigWriter &cfg) {
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
