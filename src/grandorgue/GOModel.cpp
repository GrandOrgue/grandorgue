/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOModel.h"

#include "GODefinitionFile.h"
#include "GODivisionalCoupler.h"
#include "GOEnclosure.h"
#include "GOGeneral.h"
#include "GOManual.h"
#include "GOPiston.h"
#include "GORank.h"
#include "GOSwitch.h"
#include "GOTremulant.h"
#include "GOWindchest.h"
#include "config/GOConfigReader.h"

GOModel::GOModel()
    : m_windchest(),
      m_manual(),
      m_enclosure(),
      m_switches(),
      m_tremulant(),
      m_ranks(),
      m_piston(),
      m_divisionalcoupler(),
      m_general(),
      m_FirstManual(0),
      m_ODFManualCount(0),
      m_ODFRankCount(0) {}

GOModel::~GOModel() {}

void GOModel::Load(GOConfigReader& cfg, GODefinitionFile* organfile) {
  wxString group = wxT("Organ");
  unsigned NumberOfWindchestGroups =
      cfg.ReadInteger(ODFSetting, group, wxT("NumberOfWindchestGroups"), 1, 50);

  m_windchest.resize(0);
  for (unsigned i = 0; i < NumberOfWindchestGroups; i++)
    m_windchest.push_back(new GOWindchest(organfile));

  m_ODFManualCount =
      cfg.ReadInteger(ODFSetting, group, wxT("NumberOfManuals"), 1, 16) + 1;
  m_FirstManual = cfg.ReadBoolean(ODFSetting, group, wxT("HasPedals")) ? 0 : 1;

  m_manual.resize(0);
  m_manual.resize(m_FirstManual);  // Add empty slot for pedal, if necessary
  for (unsigned int i = m_FirstManual; i < m_ODFManualCount; i++)
    m_manual.push_back(new GOManual(organfile));
  for (unsigned int i = 0; i < 4; i++)
    m_manual.push_back(new GOManual(organfile));

  unsigned NumberOfEnclosures =
      cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, 50);
  m_enclosure.resize(0);
  for (unsigned i = 0; i < NumberOfEnclosures; i++) {
    m_enclosure.push_back(new GOEnclosure(organfile));
    m_enclosure[i]->Load(cfg, wxString::Format(wxT("Enclosure%03u"), i + 1), i);
  }

  unsigned NumberOfSwitches =
      cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSwitches"), 0, 999, 0);
  m_switches.resize(0);
  for (unsigned i = 0; i < NumberOfSwitches; i++) {
    m_switches.push_back(new GOSwitch(organfile));
    m_switches[i]->Load(cfg, wxString::Format(wxT("Switch%03d"), i + 1));
  }

  unsigned NumberOfTremulants =
      cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, 10);
  for (unsigned i = 0; i < NumberOfTremulants; i++) {
    m_tremulant.push_back(new GOTremulant(organfile));
    m_tremulant[i]->Load(cfg, wxString::Format(wxT("Tremulant%03d"), i + 1),
                         -((int)(i + 1)));
  }

  for (unsigned i = 0; i < NumberOfWindchestGroups; i++)
    m_windchest[i]->Load(cfg,
                         wxString::Format(wxT("WindchestGroup%03d"), i + 1), i);

  m_ODFRankCount =
      cfg.ReadInteger(ODFSetting, group, wxT("NumberOfRanks"), 0, 999, false);
  for (unsigned i = 0; i < m_ODFRankCount; i++) {
    m_ranks.push_back(new GORank(organfile));
    m_ranks[i]->Load(cfg, wxString::Format(wxT("Rank%03d"), i + 1), -1);
  }

  for (unsigned int i = m_FirstManual; i < m_ODFManualCount; i++)
    m_manual[i]->Load(cfg, wxString::Format(wxT("Manual%03d"), i), i);

  unsigned min_key = 0xff, max_key = 0;
  for (unsigned i = GetFirstManualIndex(); i < GetODFManualCount(); i++) {
    GOManual* manual = GetManual(i);
    if ((unsigned)manual->GetFirstLogicalKeyMIDINoteNumber() < min_key)
      min_key = manual->GetFirstLogicalKeyMIDINoteNumber();
    if (manual->GetFirstLogicalKeyMIDINoteNumber() +
            manual->GetLogicalKeyCount() >
        max_key)
      max_key = manual->GetFirstLogicalKeyMIDINoteNumber() +
                manual->GetLogicalKeyCount();
  }
  for (unsigned i = GetODFManualCount(); i <= GetManualAndPedalCount(); i++)
    GetManual(i)->Init(cfg,
                       wxString::Format(wxT("SetterFloating%03d"),
                                        i - GetODFManualCount() + 1),
                       i, min_key, max_key - min_key);

  unsigned NumberOfReversiblePistons = cfg.ReadInteger(
      ODFSetting, group, wxT("NumberOfReversiblePistons"), 0, 32);
  m_piston.resize(0);
  for (unsigned i = 0; i < NumberOfReversiblePistons; i++) {
    m_piston.push_back(new GOPiston(organfile));
    m_piston[i]->Load(cfg,
                      wxString::Format(wxT("ReversiblePiston%03d"), i + 1));
  }

  unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
      ODFSetting, group, wxT("NumberOfDivisionalCouplers"), 0, 8);
  m_divisionalcoupler.resize(0);
  for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++) {
    m_divisionalcoupler.push_back(new GODivisionalCoupler(organfile));
    m_divisionalcoupler[i]->Load(
        cfg, wxString::Format(wxT("DivisionalCoupler%03d"), i + 1));
  }

  organfile->GetGeneralTemplate().InitGeneral();
  unsigned NumberOfGenerals =
      cfg.ReadInteger(ODFSetting, group, wxT("NumberOfGenerals"), 0, 99);
  m_general.resize(0);
  for (unsigned i = 0; i < NumberOfGenerals; i++) {
    m_general.push_back(
        new GOGeneral(organfile->GetGeneralTemplate(), organfile, false));
    m_general[i]->Load(cfg, wxString::Format(wxT("General%03d"), i + 1));
  }
}

void GOModel::UpdateTremulant(GOTremulant* tremulant) {
  for (unsigned i = 0; i < m_windchest.size(); i++)
    m_windchest[i]->UpdateTremulant(tremulant);
}

void GOModel::UpdateVolume() {
  for (unsigned i = 0; i < m_windchest.size(); i++)
    m_windchest[i]->UpdateVolume();
}

GOWindchest* GOModel::GetWindchest(unsigned index) {
  return m_windchest[index];
}

unsigned GOModel::GetWindchestGroupCount() { return m_windchest.size(); }

unsigned GOModel::AddWindchest(GOWindchest* windchest) {
  m_windchest.push_back(windchest);
  return m_windchest.size();
}

unsigned GOModel::GetManualAndPedalCount() {
  if (!m_manual.size()) return 0;
  return m_manual.size() - 1;
}

unsigned GOModel::GetODFManualCount() { return m_ODFManualCount; }

unsigned GOModel::GetFirstManualIndex() { return m_FirstManual; }

GOManual* GOModel::GetManual(unsigned index) { return m_manual[index]; }

unsigned GOModel::GetStopCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
    cnt += m_manual[i]->GetStopCount();
  return cnt;
}

unsigned GOModel::GetCouplerCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
    cnt += m_manual[i]->GetCouplerCount();
  return cnt;
}

unsigned GOModel::GetODFCouplerCount() {
  unsigned cnt = 0;
  for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
    cnt += m_manual[i]->GetODFCouplerCount();
  return cnt;
}

GOEnclosure* GOModel::GetEnclosureElement(unsigned index) {
  return m_enclosure[index];
}

unsigned GOModel::GetEnclosureCount() { return m_enclosure.size(); }

unsigned GOModel::AddEnclosure(GOEnclosure* enclosure) {
  m_enclosure.push_back(enclosure);
  return m_enclosure.size() - 1;
}

unsigned GOModel::GetSwitchCount() { return m_switches.size(); }

GOSwitch* GOModel::GetSwitch(unsigned index) { return m_switches[index]; }

unsigned GOModel::GetTremulantCount() { return m_tremulant.size(); }

GOTremulant* GOModel::GetTremulant(unsigned index) {
  return m_tremulant[index];
}

GORank* GOModel::GetRank(unsigned index) { return m_ranks[index]; }

unsigned GOModel::GetODFRankCount() { return m_ODFRankCount; }

void GOModel::AddRank(GORank* rank) { m_ranks.push_back(rank); }

unsigned GOModel::GetNumberOfReversiblePistons() { return m_piston.size(); }

GOPiston* GOModel::GetPiston(unsigned index) { return m_piston[index]; }

unsigned GOModel::GetDivisionalCouplerCount() {
  return m_divisionalcoupler.size();
}

GODivisionalCoupler* GOModel::GetDivisionalCoupler(unsigned index) {
  return m_divisionalcoupler[index];
}

unsigned GOModel::GetGeneralCount() { return m_general.size(); }

GOGeneral* GOModel::GetGeneral(unsigned index) { return m_general[index]; }
