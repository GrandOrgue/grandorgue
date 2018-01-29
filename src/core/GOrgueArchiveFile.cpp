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

#include "GOrgueArchiveFile.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueHash.h"
#include "GOrgueOrganList.h"
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>

GOrgueArchiveFile::GOrgueArchiveFile(wxString id, wxString path, wxString name, const std::vector<wxString>& dependencies, const std::vector<wxString>& dependency_titles) :
	m_ID(id),
	m_Path(path),
	m_Name(name),
	m_Dependencies(dependencies),
	m_DependencyTitles(dependency_titles)
{
	m_FileID = GetCurrentFileID();
}

GOrgueArchiveFile::GOrgueArchiveFile(GOrgueConfigReader& cfg, wxString group)
{
	m_ID = cfg.ReadString(CMBSetting, group, wxT("ID"));
	m_Path = cfg.ReadString(CMBSetting, group, wxT("Path"));
	m_Name = cfg.ReadString(CMBSetting, group, wxT("Name"));
	m_Dependencies.resize(cfg.ReadInteger(CMBSetting, group, wxT("DependenciesCount"), 0, 999));
	m_FileID = cfg.ReadString(CMBSetting, group, wxT("FileID"));
	for(unsigned i = 0; i < m_Dependencies.size(); i++)
	{
		m_Dependencies[i] = cfg.ReadString(CMBSetting, group, wxString::Format("Dependency%03d", i + 1));
		m_DependencyTitles[i] = cfg.ReadString(CMBSetting, group, wxString::Format("DependencyTitle%03d", i + 1));
	}
}

GOrgueArchiveFile::~GOrgueArchiveFile()
{
}

void GOrgueArchiveFile::Save(GOrgueConfigWriter& cfg, wxString group)
{
	cfg.WriteString(group, wxT("ID"), m_ID);
	cfg.WriteString(group, wxT("Path"), m_Path);
	cfg.WriteString(group, wxT("Name"), m_Name);
	cfg.WriteString(group, wxT("FileID"), m_FileID);
	cfg.WriteInteger(group, wxT("DependenciesCount"), m_Dependencies.size());
	for(unsigned i = 0; i < m_Dependencies.size(); i++)
	{
		cfg.WriteString(group, wxString::Format("Dependency%03d", i + 1), m_Dependencies[i]);
		cfg.WriteString(group, wxString::Format("DependencyTitle%03d", i + 1), m_DependencyTitles[i]);
	}
}

void GOrgueArchiveFile::Update(const GOrgueArchiveFile& archive)
{
	m_ID = archive.m_ID;
	if (m_Name != archive.m_Name)
		wxLogError(_("Organ package %s changed its title"), m_ID.c_str());
	m_Name = archive.m_Name;
	m_FileID = archive.m_FileID;
	m_Dependencies = archive.m_Dependencies;
}

wxString GOrgueArchiveFile::GetCurrentFileID() const
{
	GOrgueHash hash;
	wxFileName path_name(m_Path);

	uint64_t size = path_name.GetSize().GetValue();
	uint64_t time = path_name.GetModificationTime().GetTicks();
	hash.Update(time);
	hash.Update(size);

	return hash.getStringHash();
}

const wxString& GOrgueArchiveFile::GetID() const
{
	return m_ID;
}

const wxString& GOrgueArchiveFile::GetPath() const
{
	return m_Path;
}

const wxString& GOrgueArchiveFile::GetName() const
{
	return m_Name;
}

const wxString& GOrgueArchiveFile::GetFileID() const
{
	return m_FileID;
}

const std::vector<wxString>& GOrgueArchiveFile::GetDependencies() const
{
	return m_Dependencies;
}

const std::vector<wxString>& GOrgueArchiveFile::GetDependencyTitles() const
{
	return m_DependencyTitles;
}

bool GOrgueArchiveFile::IsUsable(const GOrgueOrganList& organs) const
{
	return wxFileExists(m_Path);
}

bool GOrgueArchiveFile::IsComplete(const GOrgueOrganList& organs) const
{
	if (!IsUsable(organs))
		return false;
	for(unsigned i = 0; i < m_Dependencies.size(); i++)
	{
		const GOrgueArchiveFile* archive = organs.GetArchiveByID(m_Dependencies[i], true);
		if (!archive)
			return false;
		if (!archive->IsUsable(organs))
			return false;
	}
	return true;
}

const wxString GOrgueArchiveFile::GetArchiveHash() const
{
	GOrgueHash hash;
	hash.Update(m_Path);
	return hash.getStringHash();
}
