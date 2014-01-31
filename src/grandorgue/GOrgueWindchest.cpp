/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueWindchest.h"

#include "GOrgueConfigReader.h"
#include "GOrgueEnclosure.h"
#include "GOrguePipe.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrgueWindchest::GOrgueWindchest(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Name(),
	m_enclosure(0),
	m_tremulant(0),
	m_ranks(0),
	m_pipes(0)
{
}


void GOrgueWindchest::Load(GOrgueConfigReader& cfg, wxString group, unsigned index)
{
	unsigned NumberOfEnclosures = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, m_organfile->GetEnclosureCount());
	unsigned NumberOfTremulants = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, m_organfile->GetTremulantCount());

	m_enclosure.resize(0);
	for (unsigned i = 0; i < NumberOfEnclosures; i++)
	{
		wxString buffer;
		buffer.Printf(wxT("Enclosure%03d"), i + 1);
		m_enclosure.push_back(cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetEnclosureCount()) - 1);
	}

	m_tremulant.resize(0);
	for (unsigned i = 0; i < NumberOfTremulants; i++)
	{
		wxString buffer;
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant.push_back(cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetTremulantCount()) - 1);
	}

	m_Name = cfg.ReadString(ODFSetting, group, wxT("Name"), false, wxString::Format(_("Windchest %d"), index + 1));
}

float GOrgueWindchest::GetVolume()
{
	float f = 1.0f;
	for (unsigned i = 0; i < m_enclosure.size(); i++)
		f *= m_organfile->GetEnclosure(m_enclosure[i])->GetAttenuation();
	return f;
}

unsigned GOrgueWindchest::GetTremulantCount()
{
	return m_tremulant.size();
}

unsigned GOrgueWindchest::GetTremulantId(unsigned no)
{
	return m_tremulant[no];
}

unsigned GOrgueWindchest::GetRankCount()
{
	return m_ranks.size();
}

GOrgueRank* GOrgueWindchest::GetRank(unsigned index)
{
	return m_ranks[index];
}

void GOrgueWindchest::AddRank(GOrgueRank* rank)
{
	m_ranks.push_back(rank);
}

void GOrgueWindchest::AddPipe(GOrguePipe* pipe)
{
	m_pipes.push_back(pipe);
}

void GOrgueWindchest::AddEnclosure(unsigned index)
{
	m_enclosure.push_back(index);
}

const wxString& GOrgueWindchest::GetName()
{
	return m_Name;
}

void GOrgueWindchest::UpdateTremulant(GOrgueTremulant* tremulant)
{
	for (unsigned i = 0; i < m_tremulant.size(); i++)
		if (tremulant == m_organfile->GetTremulant(m_tremulant[i]))
		{
			GOrgueTremulant* t = m_organfile->GetTremulant(m_tremulant[i]);
			if (t->GetTremulantType() != GOWavTrem)
				continue;
			bool on = t->IsActive();
			for(unsigned j = 0; j < m_pipes.size(); j++)
				m_pipes[j]->SetTremulant(on);
			return;
		}
}
