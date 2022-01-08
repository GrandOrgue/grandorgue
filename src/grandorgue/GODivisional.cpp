/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisional.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GODefinitionFile.h"
#include "GODivisionalCoupler.h"
#include "GOManual.h"
#include "GOSetter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GODivisional::GODivisional(
  GODefinitionFile *organfile,
  GOCombinationDefinition &divisional_template,
  bool is_setter)
  : GOPushbutton(organfile),
    GOCombination(divisional_template, organfile),
    m_DivisionalNumber(0),
    m_ManualNumber(1),
    m_IsSetter(is_setter) {}

void GODivisional::Init(
  GOConfigReader &cfg,
  wxString group,
  int manualNumber,
  int divisionalNumber,
  wxString name) {
  m_DivisionalNumber = divisionalNumber;
  m_ManualNumber = manualNumber;

  m_midi.SetIndex(manualNumber);

  m_Protected = false;

  GOPushbutton::Init(cfg, group, name);
}

void GODivisional::Load(
  GOConfigReader &cfg, wxString group, int manualNumber, int divisionalNumber) {
  m_DivisionalNumber = divisionalNumber;
  m_ManualNumber = manualNumber;

  m_midi.SetIndex(manualNumber);

  m_Protected
    = cfg.ReadBoolean(ODFSetting, group, wxT("Protected"), false, false);

  GOPushbutton::Load(cfg, group);

  if (!m_IsSetter) {
    /* skip ODF settings */
    UpdateState();
    wxString buffer;
    int pos;
    std::vector<bool> used(m_State.size());
    GOManual *associatedManual = m_organfile->GetManual(m_ManualNumber);
    unsigned NumberOfStops = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfStops"),
      0,
      associatedManual->GetStopCount(),
      true,
      0);
    unsigned NumberOfCouplers = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfCouplers"),
      0,
      associatedManual->GetODFCouplerCount(),
      m_organfile->DivisionalsStoreIntermanualCouplers()
        || m_organfile->DivisionalsStoreIntramanualCouplers(),
      0);
    unsigned NumberOfTremulants = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfTremulants"),
      0,
      m_organfile->GetTremulantCount(),
      m_organfile->DivisionalsStoreTremulants(),
      0);
    unsigned NumberOfSwitches = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfSwitches"),
      0,
      m_organfile->GetSwitchCount(),
      false,
      0);

    for (unsigned i = 0; i < NumberOfStops; i++) {
      buffer.Printf(wxT("Stop%03d"), i + 1);
      unsigned cnt = associatedManual->GetStopCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_STOP, m_ManualNumber, abs(s));
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
      buffer.Printf(wxT("Coupler%03d"), i + 1);
      unsigned cnt = associatedManual->GetODFCouplerCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_COUPLER, m_ManualNumber, abs(s));
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
      buffer.Printf(wxT("Tremulant%03d"), i + 1);
      unsigned cnt = associatedManual->GetTremulantCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt, false, 0);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_TREMULANT, m_ManualNumber, abs(s));
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
      buffer.Printf(wxT("Switch%03d"), i + 1);
      unsigned cnt = associatedManual->GetSwitchCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt, false, 0);
      pos = m_Template.findEntry(
        GOCombinationDefinition::COMBINATION_SWITCH, m_ManualNumber, abs(s));
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

void GODivisional::LoadCombination(GOConfigReader &cfg) {
  GOSettingType type = CMBSetting;
  GOManual *associatedManual = m_organfile->GetManual(m_ManualNumber);
  if (!m_IsSetter)
    if (
      cfg.ReadInteger(
        CMBSetting,
        m_group,
        wxT("NumberOfStops"),
        -1,
        associatedManual->GetStopCount(),
        false,
        -1)
      == -1)
      type = ODFSetting;
  wxString buffer;
  unsigned NumberOfStops = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfStops"),
    0,
    associatedManual->GetStopCount(),
    true,
    0);
  unsigned NumberOfCouplers = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfCouplers"),
    0,
    type == CMBSetting ? associatedManual->GetCouplerCount()
                       : associatedManual->GetODFCouplerCount(),
    m_organfile->DivisionalsStoreIntermanualCouplers()
      || m_organfile->DivisionalsStoreIntramanualCouplers(),
    0);
  unsigned NumberOfTremulants = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    m_organfile->GetTremulantCount(),
    m_organfile->DivisionalsStoreTremulants(),
    0);
  unsigned NumberOfSwitches = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    m_organfile->GetSwitchCount(),
    false,
    0);

  int pos;
  Clear();

  for (unsigned i = 0; i < NumberOfStops; i++) {
    buffer.Printf(wxT("Stop%03d"), i + 1);
    unsigned cnt = associatedManual->GetStopCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_STOP, m_ManualNumber, abs(s));
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
    buffer.Printf(wxT("Coupler%03d"), i + 1);
    unsigned cnt = type == CMBSetting ? associatedManual->GetCouplerCount()
                                      : associatedManual->GetODFCouplerCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_COUPLER, m_ManualNumber, abs(s));
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
    buffer.Printf(wxT("Tremulant%03d"), i + 1);
    unsigned cnt = associatedManual->GetTremulantCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt, false, 0);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_TREMULANT, m_ManualNumber, abs(s));
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
    buffer.Printf(wxT("Switch%03d"), i + 1);
    unsigned cnt = associatedManual->GetSwitchCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt, false, 0);
    pos = m_Template.findEntry(
      GOCombinationDefinition::COMBINATION_SWITCH, m_ManualNumber, abs(s));
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

void GODivisional::Save(GOConfigWriter &cfg) {
  wxString buffer;
  const std::vector<GOCombinationDefinition::CombinationSlot> &elements
    = m_Template.GetCombinationElements();

  GOPushbutton::Save(cfg);
  UpdateState();

  unsigned stop_count = 0;
  unsigned coupler_count = 0;
  unsigned tremulant_count = 0;
  unsigned switch_count = 0;

  for (unsigned i = 0; i < elements.size(); i++) {
    if (m_State[i] == -1) continue;
    int value = m_State[i] == 1 ? elements[i].index : -elements[i].index;
    switch (elements[i].type) {
      case GOCombinationDefinition::COMBINATION_STOP:
        stop_count++;
        buffer.Printf(wxT("Stop%03d"), stop_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_COUPLER:
        coupler_count++;
        buffer.Printf(wxT("Coupler%03d"), coupler_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_TREMULANT:
        tremulant_count++;
        buffer.Printf(wxT("Tremulant%03d"), tremulant_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_SWITCH:
        switch_count++;
        buffer.Printf(wxT("Switch%03d"), switch_count);
        cfg.WriteInteger(m_group, buffer, value);
        break;

      case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
        break;
    }
  }

  cfg.WriteInteger(m_group, wxT("NumberOfStops"), stop_count);
  cfg.WriteInteger(m_group, wxT("NumberOfCouplers"), coupler_count);
  cfg.WriteInteger(m_group, wxT("NumberOfTremulants"), tremulant_count);
  cfg.WriteInteger(m_group, wxT("NumberOfSwitches"), switch_count);
}

bool GODivisional::PushLocal() {
  bool used = GOCombination::PushLocal();
  GOManual *associatedManual = m_organfile->GetManual(m_ManualNumber);

  for (unsigned k = 0; k < associatedManual->GetDivisionalCount(); k++) {
    GODivisional *divisional = associatedManual->GetDivisional(k);
    divisional->Display(divisional == this && used);
  }
  return used;
}

void GODivisional::Push() {
  PushLocal();

  /* only use divisional couples, if not in setter mode */
  if (m_organfile->GetSetter()->IsSetterActive()) return;

  for (unsigned k = 0; k < m_organfile->GetDivisionalCouplerCount(); k++) {
    GODivisionalCoupler *coupler = m_organfile->GetDivisionalCoupler(k);
    if (!coupler->IsEngaged()) continue;

    for (unsigned i = 0; i < coupler->GetNumberOfManuals(); i++) {
      if (coupler->GetManual(i) != m_ManualNumber) continue;

      for (unsigned int j = i + 1; j < coupler->GetNumberOfManuals(); j++)
        m_organfile->GetManual(coupler->GetManual(j))
          ->GetDivisional(m_DivisionalNumber)
          ->PushLocal();

      if (coupler->IsBidirectional()) {
        for (unsigned j = 0; j < coupler->GetNumberOfManuals(); j++) {
          if (coupler->GetManual(j) == m_ManualNumber) break;
          m_organfile->GetManual(coupler->GetManual(j))
            ->GetDivisional(m_DivisionalNumber)
            ->PushLocal();
        }
      }
      break;
    }
  }
}

wxString GODivisional::GetMidiType() { return _("Divisional"); }
