/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganModel.h"

#include "combinations/control/GOGeneralButtonControl.h"
#include "config/GOConfigReader.h"
#include "control/GOPistonControl.h"
#include "modification/GOModificationListener.h"

#include "GODivisionalCoupler.h"
#include "GOEnclosure.h"
#include "GOManual.h"
#include "GOOrganController.h"
#include "GORank.h"
#include "GOSwitch.h"
#include "GOTremulant.h"
#include "GOWindchest.h"

GOOrganModel::GOOrganModel(GOConfig &config)
  : m_config(config),
    m_DivisionalsStoreIntermanualCouplers(false),
    m_DivisionalsStoreIntramanualCouplers(false),
    m_DivisionalsStoreTremulants(false),
    m_GeneralsStoreDivisionalCouplers(false),
    m_CombinationsStoreNonDisplayedDrawstops(false),
    m_RootPipeConfigNode(nullptr, this, nullptr),
    m_OrganModelModified(false),
    m_FirstManual(0),
    m_ODFManualCount(0),
    m_ODFRankCount(0) {}

GOOrganModel::~GOOrganModel() {}

void GOOrganModel::Load(
  GOConfigReader &cfg, GOOrganController *organController) {
  wxString group = wxT("Organ");
  m_DivisionalsStoreIntermanualCouplers = cfg.ReadBoolean(
    ODFSetting, group, wxT("DivisionalsStoreIntermanualCouplers"));
  m_DivisionalsStoreIntramanualCouplers = cfg.ReadBoolean(
    ODFSetting, group, wxT("DivisionalsStoreIntramanualCouplers"));
  m_DivisionalsStoreTremulants
    = cfg.ReadBoolean(ODFSetting, group, wxT("DivisionalsStoreTremulants"));
  m_GeneralsStoreDivisionalCouplers = cfg.ReadBoolean(
    ODFSetting, group, wxT("GeneralsStoreDivisionalCouplers"));
  m_CombinationsStoreNonDisplayedDrawstops = cfg.ReadBoolean(
    ODFSetting,
    group,
    wxT("CombinationsStoreNonDisplayedDrawstops"),
    false,
    true);

  unsigned NumberOfWindchestGroups = cfg.ReadInteger(
    ODFSetting, group, wxT("NumberOfWindchestGroups"), 1, 999);

  m_RootPipeConfigNode.Load(cfg, group, wxEmptyString);
  m_windchests.resize(0);
  for (unsigned i = 0; i < NumberOfWindchestGroups; i++)
    m_windchests.push_back(new GOWindchest(organController));

  m_ODFManualCount
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfManuals"), 1, 16) + 1;
  m_FirstManual = cfg.ReadBoolean(ODFSetting, group, wxT("HasPedals")) ? 0 : 1;

  m_manuals.resize(0);
  m_manuals.resize(m_FirstManual); // Add empty slot for pedal, if necessary
  for (unsigned int i = m_FirstManual; i < m_ODFManualCount; i++)
    m_manuals.push_back(new GOManual(organController));

  for (unsigned int i = 0; i < 4; i++)
    m_manuals.push_back(new GOManual(organController));

  unsigned NumberOfEnclosures
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, 999);
  m_enclosures.resize(0);
  for (unsigned i = 0; i < NumberOfEnclosures; i++) {
    m_enclosures.push_back(new GOEnclosure(*organController));
    m_enclosures[i]->Load(
      cfg, wxString::Format(wxT("Enclosure%03u"), i + 1), i);
  }

  unsigned NumberOfSwitches
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSwitches"), 0, 999, 0);
  m_switches.resize(0);
  for (unsigned i = 0; i < NumberOfSwitches; i++) {
    m_switches.push_back(new GOSwitch(organController));
    m_switches[i]->Load(cfg, wxString::Format(wxT("Switch%03d"), i + 1));
  }

  unsigned NumberOfTremulants
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, 10);
  for (unsigned i = 0; i < NumberOfTremulants; i++) {
    m_tremulants.push_back(new GOTremulant(organController));
    m_tremulants[i]->Load(
      cfg, wxString::Format(wxT("Tremulant%03d"), i + 1), -((int)(i + 1)));
  }

  for (unsigned i = 0; i < NumberOfWindchestGroups; i++)
    m_windchests[i]->Load(
      cfg, wxString::Format(wxT("WindchestGroup%03d"), i + 1), i);

  m_ODFRankCount
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfRanks"), 0, 999, false);
  for (unsigned i = 0; i < m_ODFRankCount; i++) {
    m_ranks.push_back(new GORank(*organController));
    m_ranks[i]->Load(cfg, wxString::Format(wxT("Rank%03d"), i + 1), -1);
  }

  for (unsigned int i = m_FirstManual; i < m_ODFManualCount; i++)
    m_manuals[i]->Load(cfg, wxString::Format(wxT("Manual%03d"), i), i);

  unsigned min_key = 0xff, max_key = 0;
  for (unsigned i = GetFirstManualIndex(); i < GetODFManualCount(); i++) {
    GOManual *manual = GetManual(i);
    if ((unsigned)manual->GetFirstLogicalKeyMIDINoteNumber() < min_key)
      min_key = manual->GetFirstLogicalKeyMIDINoteNumber();
    if (
      manual->GetFirstLogicalKeyMIDINoteNumber() + manual->GetLogicalKeyCount()
      > max_key)
      max_key = manual->GetFirstLogicalKeyMIDINoteNumber()
        + manual->GetLogicalKeyCount();
  }
  for (unsigned i = GetODFManualCount(); i <= GetManualAndPedalCount(); i++)
    GetManual(i)->Init(
      cfg,
      wxString::Format(wxT("SetterFloating%03d"), i - GetODFManualCount() + 1),
      i,
      min_key,
      max_key - min_key);

  unsigned NumberOfReversiblePistons = cfg.ReadInteger(
    ODFSetting, group, wxT("NumberOfReversiblePistons"), 0, 32);
  m_pistons.resize(0);
  for (unsigned i = 0; i < NumberOfReversiblePistons; i++) {
    m_pistons.push_back(new GOPistonControl(organController));
    m_pistons[i]->Load(
      cfg, wxString::Format(wxT("ReversiblePiston%03d"), i + 1));
  }

  unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
    ODFSetting, group, wxT("NumberOfDivisionalCouplers"), 0, 8);
  m_DivisionalCoupler.resize(0);
  for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++) {
    m_DivisionalCoupler.push_back(new GODivisionalCoupler(organController));
    m_DivisionalCoupler[i]->Load(
      cfg, wxString::Format(wxT("DivisionalCoupler%03d"), i + 1));
  }

  organController->GetGeneralTemplate().InitGeneral();
  unsigned NumberOfGenerals
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfGenerals"), 0, 99);
  m_generals.resize(0);
  for (unsigned i = 0; i < NumberOfGenerals; i++) {
    m_generals.push_back(new GOGeneralButtonControl(
      organController->GetGeneralTemplate(), organController, false));
    m_generals[i]->Load(cfg, wxString::Format(wxT("General%03d"), i + 1));
  }
}

void GOOrganModel::SetOrganModelModified(bool modified) {
  if (modified != m_OrganModelModified)
    m_OrganModelModified = modified;
  if (modified)
    m_ModificationProxy.OnIsModifiedChanged(modified);
}

void GOOrganModel::UpdateTremulant(GOTremulant *tremulant) {
  for (unsigned i = 0; i < m_windchests.size(); i++)
    m_windchests[i]->UpdateTremulant(tremulant);
}

void GOOrganModel::UpdateVolume() {
  for (unsigned i = 0; i < m_windchests.size(); i++)
    m_windchests[i]->UpdateVolume();
}

GOWindchest *GOOrganModel::GetWindchest(unsigned index) {
  return m_windchests[index];
}

unsigned GOOrganModel::GetWindchestGroupCount() { return m_windchests.size(); }

unsigned GOOrganModel::AddWindchest(GOWindchest *windchest) {
  m_windchests.push_back(windchest);
  return m_windchests.size();
}

unsigned GOOrganModel::GetManualAndPedalCount() {
  if (!m_manuals.size())
    return 0;
  return m_manuals.size() - 1;
}

unsigned GOOrganModel::GetODFManualCount() { return m_ODFManualCount; }

unsigned GOOrganModel::GetFirstManualIndex() { return m_FirstManual; }

GOManual *GOOrganModel::GetManual(unsigned index) { return m_manuals[index]; }

unsigned GOOrganModel::GetStopCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    cnt += m_manuals[i]->GetStopCount();
  return cnt;
}

unsigned GOOrganModel::GetCouplerCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    cnt += m_manuals[i]->GetCouplerCount();
  return cnt;
}

unsigned GOOrganModel::GetODFCouplerCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    cnt += m_manuals[i]->GetODFCouplerCount();
  return cnt;
}

GOEnclosure *GOOrganModel::GetEnclosureElement(unsigned index) {
  return m_enclosures[index];
}

unsigned GOOrganModel::GetEnclosureCount() { return m_enclosures.size(); }

unsigned GOOrganModel::AddEnclosure(GOEnclosure *enclosure) {
  m_enclosures.push_back(enclosure);
  return m_enclosures.size() - 1;
}

int GOOrganModel::FindSwitchByName(const wxString &name) const {
  int resIndex = -1;

  for (unsigned l = m_switches.size(), i = 0; i < l; i++)
    if (m_switches[i]->GetName() == name) {
      resIndex = i;
      break;
    }
  return resIndex;
}

int GOOrganModel::FindTremulantByName(const wxString &name) const {
  int resIndex = -1;

  for (unsigned l = m_tremulants.size(), i = 0; i < l; i++)
    if (m_tremulants[i]->GetName() == name) {
      resIndex = i;
      break;
    }
  return resIndex;
}

GORank *GOOrganModel::GetRank(unsigned index) { return m_ranks[index]; }

unsigned GOOrganModel::GetODFRankCount() { return m_ODFRankCount; }

void GOOrganModel::AddRank(GORank *rank) { m_ranks.push_back(rank); }

unsigned GOOrganModel::GetNumberOfReversiblePistons() {
  return m_pistons.size();
}

GOPistonControl *GOOrganModel::GetPiston(unsigned index) {
  return m_pistons[index];
}

int GOOrganModel::FindDivisionalCouplerByName(const wxString &name) const {
  int resIndex = -1;

  for (unsigned l = m_DivisionalCoupler.size(), i = 0; i < l; i++)
    if (m_DivisionalCoupler[i]->GetName() == name) {
      resIndex = i;
      break;
    }
  return resIndex;
}

unsigned GOOrganModel::GetGeneralCount() { return m_generals.size(); }

GOGeneralButtonControl *GOOrganModel::GetGeneral(unsigned index) {
  return m_generals[index];
}
