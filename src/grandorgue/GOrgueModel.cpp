/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueModel.h"

#include "GOrgueConfigReader.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueEnclosure.h"
#include "GOrgueGeneral.h"
#include "GOrgueManual.h"
#include "GOrguePiston.h"
#include "GOrgueRank.h"
#include "GOrgueSwitch.h"
#include "GOrgueTremulant.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

GOrgueModel::GOrgueModel() :
	m_windchest(),
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
	m_ODFRankCount(0)
{
}

GOrgueModel::~GOrgueModel()
{
}

void GOrgueModel::Load(GOrgueConfigReader& cfg, GrandOrgueFile* organfile)
{
	wxString group = wxT("Organ");
	unsigned NumberOfWindchestGroups = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfWindchestGroups"), 1, 50);

	m_windchest.resize(0);
	for (unsigned i = 0; i < NumberOfWindchestGroups; i++)
		m_windchest.push_back(new GOrgueWindchest(organfile));

	m_ODFManualCount = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfManuals"), 1, 16) + 1;
	m_FirstManual = cfg.ReadBoolean(ODFSetting, group, wxT("HasPedals")) ? 0 : 1;

	m_manual.resize(0);
	m_manual.resize(m_FirstManual); // Add empty slot for pedal, if necessary
	for (unsigned int i = m_FirstManual; i <  m_ODFManualCount; i++)
		m_manual.push_back(new GOrgueManual(organfile));
	for(unsigned int i = 0; i < 4; i++)
		m_manual.push_back(new GOrgueManual(organfile));

	unsigned NumberOfEnclosures = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, 50);
	m_enclosure.resize(0);
	for (unsigned i = 0; i < NumberOfEnclosures; i++)
	{
		m_enclosure.push_back(new GOrgueEnclosure(organfile));
		m_enclosure[i]->Load(cfg, wxString::Format(wxT("Enclosure%03u"), i + 1), i);
	}

	unsigned NumberOfSwitches = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSwitches"), 0, 999, 0);
	m_switches.resize(0);
	for (unsigned i = 0; i < NumberOfSwitches; i++)
	{
		m_switches.push_back(new GOrgueSwitch(organfile));
		m_switches[i]->Load(cfg, wxString::Format(wxT("Switch%03d"), i + 1));
	}

	unsigned NumberOfTremulants = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, 10);
	for (unsigned i = 0; i < NumberOfTremulants; i++)
	{
		m_tremulant.push_back(new GOrgueTremulant(organfile));
		m_tremulant[i]->Load(cfg, wxString::Format(wxT("Tremulant%03d"), i + 1), -((int)(i + 1)));
	}

	for (unsigned  i = 0; i < NumberOfWindchestGroups; i++)
		m_windchest[i]->Load(cfg, wxString::Format(wxT("WindchestGroup%03d"), i + 1), i);

	m_ODFRankCount = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfRanks"), 0, 400, false);
	for (unsigned  i = 0; i < m_ODFRankCount; i++)
	{
		m_ranks.push_back(new GOrgueRank(organfile));
		m_ranks[i]->Load(cfg, wxString::Format(wxT("Rank%03d"), i + 1), -1);
	}

	for (unsigned int i = m_FirstManual; i < m_ODFManualCount; i++)
		m_manual[i]->Load(cfg, wxString::Format(wxT("Manual%03d"), i), i);

	unsigned min_key = 0xff, max_key = 0;
	for(unsigned i = GetFirstManualIndex(); i < GetODFManualCount(); i++)
	{
		GOrgueManual* manual = GetManual(i);
		if ((unsigned)manual->GetFirstLogicalKeyMIDINoteNumber() < min_key)
			min_key = manual->GetFirstLogicalKeyMIDINoteNumber();
		if (manual->GetFirstLogicalKeyMIDINoteNumber() + manual->GetLogicalKeyCount() > max_key)
			max_key = manual->GetFirstLogicalKeyMIDINoteNumber() + manual->GetLogicalKeyCount();
	}
	for (unsigned i = GetODFManualCount(); i <= GetManualAndPedalCount(); i++)
		GetManual(i)->Init(cfg, wxString::Format(wxT("SetterFloating%03d"), i - GetODFManualCount() + 1), i, min_key, max_key - min_key);


	unsigned NumberOfReversiblePistons = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfReversiblePistons"), 0, 32);
	m_piston.resize(0);
	for (unsigned i = 0; i < NumberOfReversiblePistons; i++)
	{
		m_piston.push_back(new GOrguePiston(organfile));
		m_piston[i]->Load(cfg, wxString::Format(wxT("ReversiblePiston%03d"), i + 1));
	}

	unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfDivisionalCouplers"), 0, 8);
	m_divisionalcoupler.resize(0);
	for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++)
	{
		m_divisionalcoupler.push_back(new GOrgueDivisionalCoupler(organfile));
		m_divisionalcoupler[i]->Load(cfg, wxString::Format(wxT("DivisionalCoupler%03d"), i + 1));
	}

	organfile->GetGeneralTemplate().InitGeneral();
	unsigned NumberOfGenerals = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfGenerals"), 0, 99);
	m_general.resize(0);
	for (unsigned i = 0; i < NumberOfGenerals; i++)
	{
		m_general.push_back(new GOrgueGeneral(organfile->GetGeneralTemplate(), organfile, false));
		m_general[i]->Load(cfg, wxString::Format(wxT("General%03d"), i + 1));
	}
}

void GOrgueModel::UpdateTremulant(GOrgueTremulant* tremulant)
{
	for(unsigned i = 0; i < m_windchest.size(); i++)
		m_windchest[i]->UpdateTremulant(tremulant);
}

void GOrgueModel::UpdateVolume()
{
	for (unsigned i = 0; i < m_windchest.size(); i++)
		m_windchest[i]->UpdateVolume();
}

GOrgueWindchest* GOrgueModel::GetWindchest(unsigned index)
{
	return m_windchest[index];
}

unsigned GOrgueModel::GetWindchestGroupCount()
{
	return m_windchest.size();
}

unsigned GOrgueModel::AddWindchest(GOrgueWindchest* windchest)
{
	m_windchest.push_back(windchest);
	return m_windchest.size();
}

unsigned GOrgueModel::GetManualAndPedalCount()
{
	if (!m_manual.size())
		return 0;
	return m_manual.size() - 1;
}

unsigned GOrgueModel::GetODFManualCount()
{
	return m_ODFManualCount;
}

unsigned GOrgueModel::GetFirstManualIndex()
{
	return m_FirstManual;
}

GOrgueManual* GOrgueModel::GetManual(unsigned index)
{
	return m_manual[index];
}

GOrgueEnclosure* GOrgueModel::GetEnclosureElement(unsigned index)
{
	return m_enclosure[index];
}

unsigned GOrgueModel::GetEnclosureCount()
{
	return m_enclosure.size();
}

unsigned GOrgueModel::AddEnclosure(GOrgueEnclosure* enclosure)
{
	m_enclosure.push_back(enclosure);
	return m_enclosure.size() - 1;
}

unsigned GOrgueModel::GetSwitchCount()
{
	return m_switches.size();
}

GOrgueSwitch* GOrgueModel::GetSwitch(unsigned index)
{
	return m_switches[index];
}

unsigned GOrgueModel::GetTremulantCount()
{
	return m_tremulant.size();
}

GOrgueTremulant* GOrgueModel::GetTremulant(unsigned index)
{
	return m_tremulant[index];
}

GOrgueRank* GOrgueModel::GetRank(unsigned index)
{
	return m_ranks[index];
}

unsigned GOrgueModel::GetODFRankCount()
{
	return m_ODFRankCount;
}

void GOrgueModel::AddRank(GOrgueRank* rank)
{
	m_ranks.push_back(rank);
}

unsigned GOrgueModel::GetNumberOfReversiblePistons()
{
	return m_piston.size();
}

GOrguePiston* GOrgueModel::GetPiston(unsigned index)
{
	return m_piston[index];
}

unsigned GOrgueModel::GetDivisionalCouplerCount()
{
	return m_divisionalcoupler.size();
}

GOrgueDivisionalCoupler* GOrgueModel::GetDivisionalCoupler(unsigned index)
{
	return m_divisionalcoupler[index];
}

unsigned GOrgueModel::GetGeneralCount()
{
	return m_general.size();
}

GOrgueGeneral* GOrgueModel::GetGeneral(unsigned index)
{
	return m_general[index];
}



