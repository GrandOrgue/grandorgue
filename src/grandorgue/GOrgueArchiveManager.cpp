/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueArchiveManager.h"

#include "GOrgueArchive.h"
#include "GOrgueArchiveFile.h"
#include "GOrgueConfigFileReader.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigReaderDB.h"
#include "GOrgueFile.h"
#include "GOrgueOrgan.h"
#include "GOrguePath.h"
#include "GOrgueSettings.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>

GOrgueArchiveManager::GOrgueArchiveManager(GOrgueSettings& settings) :
	m_Settings(settings)
{
}

GOrgueArchiveManager::~GOrgueArchiveManager()
{
}

GOrgueArchive* GOrgueArchiveManager::OpenArchive(const wxString& path)
{
	GOrgueArchive* archive = new GOrgueArchive(m_Settings);
	if (!archive->OpenArchive(path))
	{
		delete archive;
		return NULL;
	}
	return archive;
}

bool GOrgueArchiveManager::ReadIndex(GOrgueArchive* archive, bool InstallOrgans)
{
	GOrgueFile* indexFile = NULL;
	if (!indexFile)
		return false;
	GOrgueConfigFileReader ini_file;
	if (!ini_file.Read(indexFile))
	{
		delete indexFile;
		return false;
	}
	delete indexFile;
	try
	{
		GOrgueConfigReaderDB ini;
		ini.ReadData(ini_file, CMBSetting, false);
		GOrgueConfigReader cfg(ini);
		wxString id = archive->GetArchiveID();

		wxString package_name = cfg.ReadString(CMBSetting, wxT("General"), wxT("Title"));
		unsigned dep_count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("DependencyCount"), 0, 100, false, 0);
		unsigned organ_count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("OrganCount"), 0, 100);

		std::vector<wxString> depend;
		std::vector<wxString> depend_titles;
		for(unsigned i = 0; i < dep_count; i++)
		{
			wxString group = wxString::Format(wxT("Dependency%03d"), i + 1);
			wxString dep_id = cfg.ReadString(CMBSetting, group, wxT("PackageID"));
			wxString title = cfg.ReadString(CMBSetting, group, wxT("Title"));
			if (dep_id == id)
				throw (wxString)_("Self referencing organ package");
			depend.push_back(dep_id);
			depend_titles.push_back(title);
		}
		archive->SetDependencies(depend);

		std::vector<GOrgueOrgan> organs;
		for(unsigned i = 0; i < organ_count; i++)
		{
			wxString group = wxString::Format(wxT("Organ%03d"), i + 1);
			wxString odf = cfg.ReadString(CMBSetting, group, wxT("Filename"));
			wxString ChurchName = cfg.ReadString(CMBSetting, group, wxT("ChurchName"));
			wxString OrganBuilder = cfg.ReadString(CMBSetting, group, wxT("OrganBuilder"),  false);
			wxString RecordingDetails = cfg.ReadString(CMBSetting, group, wxT("RecordingDetails"),  false);
			if (InstallOrgans)
				organs.push_back(GOrgueOrgan(odf, id, ChurchName, OrganBuilder, RecordingDetails));
		}

		GOrgueArchiveFile a(id, archive->GetPath(), package_name, depend, depend_titles);
		m_Settings.AddArchive(a);

		for(unsigned i = 0; i < organs.size(); i++)
			m_Settings.AddOrgan(organs[i]);
	}
	catch(wxString error)
	{
		wxLogError(wxT("%s\n"),error.c_str());
		return false;
        }
	return true;
}


GOrgueArchive* GOrgueArchiveManager::LoadArchive(const wxString& id)
{
	for(unsigned i = 0; i < m_Settings.GetArchiveList().size(); i++)
	{
		GOrgueArchiveFile* file = m_Settings.GetArchiveList()[i];
		if (file->GetID() != id)
			continue;
		GOrgueArchive* archive = OpenArchive(file->GetPath());
		if (!archive)
			continue;
		if (!ReadIndex(archive, archive->GetArchiveID() != id))
		{
			delete archive;
			continue;
		}
		if (archive->GetArchiveID() != id)
		{
			delete archive;
			continue;
		}
		return archive;
	}
	return NULL;
}

wxString GOrgueArchiveManager::InstallPackage(const wxString& path)
{
	wxString p = GONormalizePath(path);
	GOrgueArchive* archive = OpenArchive(p);
	if (!archive)
		return wxString::Format(_("Failed to open the organ package '%s'"), path.c_str());
	bool result = ReadIndex(archive, true);
	delete archive;
	if (!result)
		return wxString::Format(_("Failed to parse the organ package index of '%s'"), path.c_str());
	return wxEmptyString;
}

void GOrgueArchiveManager::RegisterPackage(const wxString& path)
{
	wxString p = GONormalizePath(path);
	if (m_Settings.GetArchiveByPath(p) != NULL)
		return;
	wxString result = InstallPackage(p);
	if (result != wxEmptyString)
		wxLogError(_("%s"), result.c_str());
}

void GOrgueArchiveManager::RegisterPackageDirectory(const wxString& path)
{
	wxDir dir(path);
	if(!dir.IsOpened())
	{
		wxLogError(_("Failed to read directory '%s'"), path.c_str());
		return;
	}
	wxString name;
	if (!dir.GetFirst(&name, wxT("*.orgue"), wxDIR_FILES | wxDIR_HIDDEN))
		return;
	do
	{
		RegisterPackage(path + wxFileName::GetPathSeparator() + name);
	}
	while(dir.GetNext(&name));
}
