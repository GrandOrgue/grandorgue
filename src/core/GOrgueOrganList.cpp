/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueOrganList.h"

#include "GOrgueArchiveFile.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueOrgan.h"
#include <algorithm>

GOrgueOrganList::GOrgueOrganList() :
	m_OrganList(),
	m_ArchiveList()
{
}
	
GOrgueOrganList::~GOrgueOrganList()
{
}

void GOrgueOrganList::Load(GOrgueConfigReader& cfg, GOrgueMidiMap& map)
{
	m_OrganList.clear();
	unsigned organ_count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("OrganCount"), 0, 99999, false, 0);
	for(unsigned i = 0; i < organ_count; i++)
		m_OrganList.push_back(new GOrgueOrgan(cfg, wxString::Format(wxT("Organ%03d"), i + 1), map));

	m_ArchiveList.clear();
	unsigned archive_count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("ArchiveCount"), 0, 99999, false, 0);
	for(unsigned i = 0; i < archive_count; i++)
		m_ArchiveList.push_back(new GOrgueArchiveFile(cfg, wxString::Format(wxT("Archive%03d"), i + 1)));
}

void GOrgueOrganList::Save(GOrgueConfigWriter& cfg, GOrgueMidiMap& map)
{
	cfg.WriteInteger(wxT("General"), wxT("ArchiveCount"), m_ArchiveList.size());
	for(unsigned i = 0; i < m_ArchiveList.size(); i++)
		m_ArchiveList[i]->Save(cfg, wxString::Format(wxT("Archive%03d"), i + 1));

	cfg.WriteInteger(wxT("General"), wxT("OrganCount"), m_OrganList.size());
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		m_OrganList[i]->Save(cfg, wxString::Format(wxT("Organ%03d"), i + 1), map);
}

const ptr_vector<GOrgueOrgan>& GOrgueOrganList::GetOrganList() const
{
	return m_OrganList;
}

ptr_vector<GOrgueOrgan>& GOrgueOrganList::GetOrganList()
{
	return m_OrganList;
}

static bool LRUCompare(const GOrgueOrgan* a, const GOrgueOrgan* b)
{
	return a->GetLastUse() > b->GetLastUse();
}

std::vector<const GOrgueOrgan*> GOrgueOrganList::GetLRUOrganList()
{
	std::vector<const GOrgueOrgan*> lru;
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		if (m_OrganList[i]->IsUsable(*this))
			lru.push_back(m_OrganList[i]);
	std::sort(lru.begin(), lru.end(), LRUCompare);
	return lru;
}

void GOrgueOrganList::AddOrgan(const GOrgueOrgan& organ)
{
	for(unsigned i = 0; i < m_OrganList.size(); i++)
		if (organ.GetODFPath() == m_OrganList[i]->GetODFPath() &&
		    organ.GetArchiveID() == m_OrganList[i]->GetArchiveID())
		{
			m_OrganList[i]->Update(organ);
			return;
		}
	m_OrganList.push_back(new GOrgueOrgan(organ));
}

const ptr_vector<GOrgueArchiveFile>& GOrgueOrganList::GetArchiveList() const
{
	return m_ArchiveList;
}

ptr_vector<GOrgueArchiveFile>& GOrgueOrganList::GetArchiveList()
{
	return m_ArchiveList;
}

const GOrgueArchiveFile* GOrgueOrganList::GetArchiveByID(const wxString& id, bool useable) const
{
	for(unsigned i = 0; i < m_ArchiveList.size(); i++)
		if (m_ArchiveList[i]->GetID() == id)
			if (!useable || m_ArchiveList[i]->IsUsable(*this))
				return m_ArchiveList[i];
	return NULL;
}

const GOrgueArchiveFile* GOrgueOrganList::GetArchiveByPath(const wxString& path) const
{
	for(unsigned i = 0; i < m_ArchiveList.size(); i++)
		if (m_ArchiveList[i]->GetPath() == path)
			return m_ArchiveList[i];
	return NULL;
}

void GOrgueOrganList::AddArchive(const GOrgueArchiveFile& archive)
{
	for(unsigned i = 0; i < m_ArchiveList.size(); i++)
		if (m_ArchiveList[i]->GetPath() == archive.GetPath())
		{
			m_ArchiveList[i]->Update(archive);
			return;
		}
	m_ArchiveList.push_back(new GOrgueArchiveFile(archive));
}

