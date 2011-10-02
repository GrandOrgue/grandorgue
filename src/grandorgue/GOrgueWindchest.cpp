/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueWindchest.h"
#include "GOrgueEnclosure.h"
#include "GrandOrgueFile.h"

GOrgueWindchest::GOrgueWindchest(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_enclosure(0),
	m_tremulant(0)
{
}


void GOrgueWindchest::Load(IniFileConfig& cfg, wxString group)
{
	unsigned NumberOfEnclosures = cfg.ReadInteger(group, wxT("NumberOfEnclosures"), 0, m_organfile->GetEnclosureCount());
	unsigned NumberOfTremulants = cfg.ReadInteger(group, wxT("NumberOfTremulants"), 0, m_organfile->GetTremulantCount());

	m_enclosure.resize(0);
	for (unsigned i = 0; i < NumberOfEnclosures; i++)
	{
		wxString buffer;
		buffer.Printf(wxT("Enclosure%03d"), i + 1);
		m_enclosure.push_back(cfg.ReadInteger(group, buffer, 1, m_organfile->GetEnclosureCount()) - 1);
	}

	m_tremulant.resize(0);
	for (unsigned i = 0; i < NumberOfTremulants; i++)
	{
		wxString buffer;
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant.push_back(cfg.ReadInteger(group, buffer, 1, m_organfile->GetTremulantCount()) - 1);
	}

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
