/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOModel.h"

#include "combinations/control/GOGeneralButtonControl.h"
#include "config/GOConfigReader.h"
#include "control/GOPistonControl.h"

#include "GODivisionalCoupler.h"
#include "GOEnclosure.h"
#include "GOManual.h"
#include "GOOrganController.h"
#include "GORank.h"
#include "GOSwitch.h"
#include "GOTremulant.h"
#include "GOWindchest.h"

GOModel::GOModel()
  : m_windchests(),
    m_manuals(),
    m_enclosures(),
    m_switches(),
    m_tremulants(),
    m_ranks(),
    m_pistons(),
    m_DivisionalCoupler(),
    m_generals(),
    m_FirstManual(0),
    m_ODFManualCount(0),
    m_ODFRankCount(0) {}

GOModel::~GOModel() {}

void GOModel::Load(GOConfigReader &cfg, GOOrganController *organController) {
  wxString group = wxT("Organ");
  unsigned NumberOfWindchestGroups
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfWindchestGroups"), 1, 50);

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
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, 50);
  m_enclosures.resize(0);
  for (unsigned i = 0; i < NumberOfEnclosures; i++) {
    m_enclosures.push_back(new GOEnclosure(organController));
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
    m_ranks.push_back(new GORank(organController));
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

void GOModel::UpdateTremulant(GOTremulant *tremulant) {
  for (unsigned i = 0; i < m_windchests.size(); i++)
    m_windchests[i]->UpdateTremulant(tremulant);
}

void GOModel::UpdateVolume() {
  for (unsigned i = 0; i < m_windchests.size(); i++)
    m_windchests[i]->UpdateVolume();
}

GOWindchest *GOModel::GetWindchest(unsigned index) {
  return m_windchests[index];
}

unsigned GOModel::GetWindchestGroupCount() { return m_windchests.size(); }

unsigned GOModel::AddWindchest(GOWindchest *windchest) {
  m_windchests.push_back(windchest);
  return m_windchests.size();
}

unsigned GOModel::GetManualAndPedalCount() {
  if (!m_manuals.size())
    return 0;
  return m_manuals.size() - 1;
}

unsigned GOModel::GetODFManualCount() { return m_ODFManualCount; }

unsigned GOModel::GetFirstManualIndex() { return m_FirstManual; }

GOManual *GOModel::GetManual(unsigned index) { return m_manuals[index]; }

unsigned GOModel::GetStopCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    cnt += m_manuals[i]->GetStopCount();
  return cnt;
}

unsigned GOModel::GetCouplerCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    cnt += m_manuals[i]->GetCouplerCount();
  return cnt;
}

unsigned GOModel::GetODFCouplerCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    cnt += m_manuals[i]->GetODFCouplerCount();
  return cnt;
}

GOEnclosure *GOModel::GetEnclosureElement(unsigned index) {
  return m_enclosures[index];
}

unsigned GOModel::GetEnclosureCount() { return m_enclosures.size(); }

unsigned GOModel::AddEnclosure(GOEnclosure *enclosure) {
  m_enclosures.push_back(enclosure);
  return m_enclosures.size() - 1;
}

unsigned GOModel::GetSwitchCount() { return m_switches.size(); }

GOSwitch *GOModel::GetSwitch(unsigned index) { return m_switches[index]; }

unsigned GOModel::GetTremulantCount() { return m_tremulants.size(); }

GOTremulant *GOModel::GetTremulant(unsigned index) {
  return m_tremulants[index];
}

GORank *GOModel::GetRank(unsigned index) { return m_ranks[index]; }

unsigned GOModel::GetODFRankCount() { return m_ODFRankCount; }

void GOModel::AddRank(GORank *rank) { m_ranks.push_back(rank); }

unsigned GOModel::GetNumberOfReversiblePistons() { return m_pistons.size(); }

GOPistonControl *GOModel::GetPiston(unsigned index) { return m_pistons[index]; }

unsigned GOModel::GetDivisionalCouplerCount() {
  return m_DivisionalCoupler.size();
}

GODivisionalCoupler *GOModel::GetDivisionalCoupler(unsigned index) {
  return m_DivisionalCoupler[index];
}

unsigned GOModel::GetGeneralCount() { return m_generals.size(); }

GOGeneralButtonControl *GOModel::GetGeneral(unsigned index) {
  return m_generals[index];
}
