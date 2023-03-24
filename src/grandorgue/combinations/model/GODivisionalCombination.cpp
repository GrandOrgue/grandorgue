/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalCombination.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "combinations/control/GODivisionalButtonControl.h"

#include "combinations/GOSetter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "yaml/go-wx-yaml.h"

#include "GOOrganController.h"

GODivisionalCombination::GODivisionalCombination(
  GOOrganController *organController,
  GOCombinationDefinition &divisional_template,
  bool is_setter)
  : GOCombination(divisional_template, organController),
    m_OrganController(organController),
    m_odfManualNumber(1),
    m_DivisionalNumber(0),
    m_IsSetter(is_setter) {}

void GODivisionalCombination::Init(
  const wxString &group, int manualNumber, int divisionalNumber) {
  m_group = group;
  m_odfManualNumber = manualNumber;
  m_DivisionalNumber = divisionalNumber;
  m_Protected = false;
}

static const wxString WX_NUMBER_OF_STOPS = wxT("NumberOfStops");

int read_number_of_stops(
  GOSettingType settingType,
  GOConfigReader &cfg,
  const wxString &group,
  unsigned maxStopN,
  bool isRequired) {
  return cfg.ReadInteger(
    settingType,
    group,
    WX_NUMBER_OF_STOPS,
    0,
    maxStopN,
    isRequired,
    isRequired ? 0 : -1);
}

void GODivisionalCombination::Load(
  GOConfigReader &cfg, wxString group, int manualNumber, int divisionalNumber) {
  Init(group, manualNumber, divisionalNumber);
  m_Protected
    = cfg.ReadBoolean(ODFSetting, group, wxT("Protected"), false, false);

  if (!m_IsSetter) {
    /* skip ODF settings */
    UpdateState();
    wxString buffer;
    int pos;
    std::vector<bool> used(m_State.size());
    GOManual *associatedManual
      = m_OrganController->GetManual(m_odfManualNumber);
    unsigned NumberOfStops = (unsigned)read_number_of_stops(
      ODFSetting, cfg, m_group, associatedManual->GetStopCount(), true);
    unsigned NumberOfCouplers = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfCouplers"),
      0,
      associatedManual->GetODFCouplerCount(),
      m_OrganController->DivisionalsStoreIntermanualCouplers()
        || m_OrganController->DivisionalsStoreIntramanualCouplers(),
      0);
    unsigned NumberOfTremulants = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfTremulants"),
      0,
      m_OrganController->GetTremulantCount(),
      m_OrganController->DivisionalsStoreTremulants(),
      0);
    unsigned NumberOfSwitches = cfg.ReadInteger(
      ODFSetting,
      m_group,
      wxT("NumberOfSwitches"),
      0,
      m_OrganController->GetSwitchCount(),
      false,
      0);

    for (unsigned i = 0; i < NumberOfStops; i++) {
      buffer.Printf(wxT("Stop%03d"), i + 1);
      unsigned cnt = associatedManual->GetStopCount();
      int s = cfg.ReadInteger(ODFSetting, m_group, buffer, -cnt, cnt);
      pos = m_Template.FindElement(
        GOCombinationDefinition::COMBINATION_STOP, m_odfManualNumber, abs(s));
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
      pos = m_Template.FindElement(
        GOCombinationDefinition::COMBINATION_COUPLER,
        m_odfManualNumber,
        abs(s));
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
      pos = m_Template.FindElement(
        GOCombinationDefinition::COMBINATION_TREMULANT,
        m_odfManualNumber,
        abs(s));
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
      pos = m_Template.FindElement(
        GOCombinationDefinition::COMBINATION_SWITCH, m_odfManualNumber, abs(s));
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

void GODivisionalCombination::LoadCombination(GOConfigReader &cfg) {
  GOSettingType type = CMBSetting;
  GOManual *associatedManual = m_OrganController->GetManual(m_odfManualNumber);
  if (!m_IsSetter)
    if (read_number_of_stops(type, cfg, m_group, 999, false) == -1)
      type = ODFSetting;
  wxString buffer;
  unsigned NumberOfStops = read_number_of_stops(
    type, cfg, m_group, associatedManual->GetStopCount(), true);
  unsigned NumberOfCouplers = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfCouplers"),
    0,
    type == CMBSetting ? associatedManual->GetCouplerCount()
                       : associatedManual->GetODFCouplerCount(),
    m_OrganController->DivisionalsStoreIntermanualCouplers()
      || m_OrganController->DivisionalsStoreIntramanualCouplers(),
    0);
  unsigned NumberOfTremulants = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfTremulants"),
    0,
    m_OrganController->GetTremulantCount(),
    m_OrganController->DivisionalsStoreTremulants(),
    0);
  unsigned NumberOfSwitches = cfg.ReadInteger(
    type,
    m_group,
    wxT("NumberOfSwitches"),
    0,
    m_OrganController->GetSwitchCount(),
    false,
    0);

  int pos;
  Clear();

  for (unsigned i = 0; i < NumberOfStops; i++) {
    buffer.Printf(wxT("Stop%03d"), i + 1);
    unsigned cnt = associatedManual->GetStopCount();
    int s = cfg.ReadInteger(type, m_group, buffer, -cnt, cnt);
    pos = m_Template.FindElement(
      GOCombinationDefinition::COMBINATION_STOP, m_odfManualNumber, abs(s));
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
    pos = m_Template.FindElement(
      GOCombinationDefinition::COMBINATION_COUPLER, m_odfManualNumber, abs(s));
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
    pos = m_Template.FindElement(
      GOCombinationDefinition::COMBINATION_TREMULANT,
      m_odfManualNumber,
      abs(s));
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
    pos = m_Template.FindElement(
      GOCombinationDefinition::COMBINATION_SWITCH, m_odfManualNumber, abs(s));
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

void GODivisionalCombination::Save(GOConfigWriter &cfg) {
  wxString buffer;
  const std::vector<GOCombinationDefinition::Element> &elements
    = m_Template.GetElements();

  UpdateState();

  unsigned stop_count = 0;
  unsigned coupler_count = 0;
  unsigned tremulant_count = 0;
  unsigned switch_count = 0;

  for (unsigned i = 0; i < elements.size(); i++) {
    if (m_State[i] == -1)
      continue;
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

  cfg.WriteInteger(m_group, WX_NUMBER_OF_STOPS, stop_count);
  cfg.WriteInteger(m_group, wxT("NumberOfCouplers"), coupler_count);
  cfg.WriteInteger(m_group, wxT("NumberOfTremulants"), tremulant_count);
  cfg.WriteInteger(m_group, wxT("NumberOfSwitches"), switch_count);
}

const wxString WX_P03D = "%03d";
const char *const STOPS = "stops";
const char *const COUPLERS = "couplers";
const char *const TREMULANTS = "tremulants";
const char *const SWITCHES = "switches";

void GODivisionalCombination::ToYaml(YAML::Node &yamlNode) const {
  const std::vector<GOCombinationDefinition::Element> &elements
    = m_Template.GetElements();
  GOManual &manual = *m_OrganController->GetManual(m_odfManualNumber);

  for (unsigned i = 0; i < elements.size(); i++)
    if (m_State[i] > 0) {
      const auto e = elements[i];
      unsigned value = e.index;
      const wxString valueLabel = wxString::Format(WX_P03D, value);

      assert(value > 0);

      unsigned index = value - 1;

      switch (e.type) {
      case GOCombinationDefinition::COMBINATION_STOP:
        yamlNode[STOPS][valueLabel] = manual.GetStop(index)->GetName();
        break;

      case GOCombinationDefinition::COMBINATION_COUPLER:
        yamlNode[COUPLERS][valueLabel] = manual.GetCoupler(index)->GetName();
        break;

      case GOCombinationDefinition::COMBINATION_TREMULANT:
        yamlNode[TREMULANTS][valueLabel]
          = m_OrganController->GetTremulant(index)->GetName();
        break;

      case GOCombinationDefinition::COMBINATION_SWITCH:
        yamlNode[SWITCHES][valueLabel]
          = m_OrganController->GetSwitch(index)->GetName();
        break;

      case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER:
        break;
      }
    }
}

void GODivisionalCombination::Push(ExtraElementsSet const *extraSet) {
  PushLocal(extraSet);

  /* only use divisional couples, if not in setter mode */
  if (m_OrganController->GetSetter()->IsSetterActive())
    return;

  for (unsigned k = 0; k < m_OrganController->GetDivisionalCouplerCount();
       k++) {
    GODivisionalCoupler *coupler = m_OrganController->GetDivisionalCoupler(k);
    if (!coupler->IsEngaged())
      continue;

    for (unsigned i = 0; i < coupler->GetNumberOfManuals(); i++) {
      if (coupler->GetManual(i) != m_odfManualNumber)
        continue;

      for (unsigned int j = i + 1; j < coupler->GetNumberOfManuals(); j++)
        m_OrganController->GetManual(coupler->GetManual(j))
          ->GetDivisional(m_DivisionalNumber)
          ->Push();

      if (coupler->IsBidirectional()) {
        for (unsigned j = 0; j < coupler->GetNumberOfManuals(); j++) {
          if (coupler->GetManual(j) == m_odfManualNumber)
            break;
          m_OrganController->GetManual(coupler->GetManual(j))
            ->GetDivisional(m_DivisionalNumber)
            ->Push();
        }
      }
      break;
    }
  }
}

wxString GODivisionalCombination::GetMidiType() { return _("Divisional"); }

// checks if a combinatiom exists in the file with the group
bool is_cmb_on_file(
  GOSettingType settingType, GOConfigReader &cfg, const wxString &group) {
  int nOfStops = read_number_of_stops(settingType, cfg, group, 999, false);

  return nOfStops != -1;
}

GODivisionalCombination *GODivisionalCombination::LoadFrom(
  GOOrganController *organController,
  GOConfigReader &cfg,
  GOCombinationDefinition &divisionalTemplate,
  const wxString &group,
  const wxString &readGroup,
  int manualNumber,
  int divisionalNumber) {
  GODivisionalCombination *pCmb = nullptr;
  bool isCmbOnFile = is_cmb_on_file(ODFSetting, cfg, group)
    || is_cmb_on_file(CMBSetting, cfg, group);
  bool isCmbOnReadGroup = !readGroup.IsEmpty()
    && (is_cmb_on_file(ODFSetting, cfg, readGroup) || is_cmb_on_file(CMBSetting, cfg, readGroup));

  if (isCmbOnFile || isCmbOnReadGroup) {
    pCmb
      = new GODivisionalCombination(organController, divisionalTemplate, false);
    pCmb->Load(
      cfg,
      isCmbOnReadGroup ? readGroup : group,
      manualNumber,
      divisionalNumber);
    pCmb->LoadCombination(cfg);
    if (isCmbOnReadGroup) {  // The combination was loaded from the legacy group
      pCmb->m_group = group; // It will be saved to the normal group
      if (isCmbOnFile)       // Load the overriden combination
        pCmb->LoadCombination(cfg);
    }
  }
  return pCmb;
}

void GODivisionalCombination::FromYaml(const YAML::Node &node) {
  throw wxT("Not implemented yet");
}
