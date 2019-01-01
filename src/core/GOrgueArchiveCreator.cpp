/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueArchiveCreator.h"

#include "GOrgueArchive.h"
#include "GOrgueArchiveFile.h"
#include "GOrgueCompress.h"
#include "GOrgueConfigFileReader.h"
#include "GOrgueConfigFileWriter.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueOrgan.h"
#include "GOrguePath.h"
#include "GOrgueStandardFile.h"
#include "GOrgueWave.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>

GOrgueArchiveCreator::GOrgueArchiveCreator(const GOrgueSettingDirectory& cacheDir) :
	m_OrganList(),
	m_Manager(m_OrganList, cacheDir),
	m_Output(),
	m_packageIDs(),
	m_packages(),
	m_organs(),
	m_OrganPaths(),
	m_PackageTitle()
{
}

GOrgueArchiveCreator::~GOrgueArchiveCreator()
{
}

bool GOrgueArchiveCreator::AddPackage(const wxString& path)
{
	if (!m_Manager.RegisterPackage(path))
	{
		wxLogError(_("Failed to index organ package %s"), path.c_str());
		return false;
	}
	const GOrgueArchiveFile* a = m_OrganList.GetArchiveByPath(path);
	if (!a)
	{
		wxLogError(_("Failed to find organ package %s"), path.c_str());
		return false;
	}
	for(unsigned i = 0; i < m_packageIDs.size(); i++)
		if (a->GetID() == m_packageIDs[i]->GetID())
		{
			wxLogWarning(_("PackageID %s added more than once"), a->GetID().c_str());
			return true;
		}
	m_packageIDs.push_back(a);
	GOrgueArchive* archive = m_Manager.LoadArchive(a->GetID());
	if (!archive)
	{
		wxLogError(_("Failed to load organ package %s"), path.c_str());
		return false;
	}
	m_packages.push_back(archive);
	return true;
}

void GOrgueArchiveCreator::AddOrgan(const wxString& path)
{
	wxString name = path;
	name.Replace(wxFileName::GetPathSeparator(), wxT("\\"));
	m_organs.push_back(NULL);
	m_OrganPaths.push_back(name);
}

std::unique_ptr<GOrgueFile> GOrgueArchiveCreator::findPackageFile(const wxString& name)
{
	for(unsigned i = 0; i < m_packages.size(); i++)
	{
		if (m_packages[i]->containsFile(name))
			return std::unique_ptr<GOrgueFile>(m_packages[i]->OpenFile(name));
	}
	return nullptr;
}

bool GOrgueArchiveCreator::storeFile(const wxString& name, const GOrgueBuffer<uint8_t>& data)
{
	std::unique_ptr<GOrgueFile> archiveFile = findPackageFile(name);
	if (archiveFile && archiveFile->GetSize() == data.GetSize())
	{
		GOrgueBuffer<uint8_t> compare;
		if (!archiveFile->ReadContent(compare))
		{
			wxLogError(_("Failed to read %s from the organ package"), name.c_str());
			return false;
		}
		if (!memcmp(data.get(), compare.get(), data.GetSize()))
			return true;
	}
	if (!m_Output.Add(name, data))
	{
		wxLogError(_("Failed to write %s to the organ package"), name.c_str());
		return false;
	}
	return true;
}

bool GOrgueArchiveCreator::CreatePackage(const wxString& path, const wxString title)
{
	m_PackageTitle = title;
	if (!m_Output.Open(path))
	{
		wxLogError(_("Failed creating organ package file %s"), path.c_str());
		return false;
	}
	return true;
}

bool GOrgueArchiveCreator::writePackageIndex()
{
	GOrgueConfigFileWriter cfg_file;
	GOrgueConfigWriter cfg(cfg_file, false);

	cfg.WriteString(wxT("General"), wxT("Title"), m_PackageTitle);

	cfg.WriteInteger(wxT("General"), wxT("DependencyCount"), m_packages.size());
	for(unsigned i = 0; i < m_packageIDs.size(); i++)
	{
		wxString group = wxString::Format(wxT("Dependency%03d"), i + 1);
		cfg.WriteString(group, wxT("PackageID"), m_packageIDs[i]->GetID());
		cfg.WriteString(group, wxT("Title"), m_packageIDs[i]->GetName());
	}

	cfg.WriteInteger(wxT("General"), wxT("OrganCount"), m_organs.size());
	for(unsigned i = 0; i < m_organs.size(); i++)
	{
		wxString group = wxString::Format(wxT("Organ%03d"), i + 1);
		cfg.WriteString(group, wxT("Filename"), m_organs[i]->GetODFPath());
		cfg.WriteString(group, wxT("ChurchName"), m_organs[i]->GetChurchName());
		cfg.WriteString(group, wxT("OrganBuilder"), m_organs[i]->GetOrganBuilder());
		cfg.WriteString(group, wxT("RecordingDetails"), m_organs[i]->GetRecordingDetail());
	}

	GOrgueBuffer<uint8_t> buf;
	if (!cfg_file.GetFileContent(buf))
		return false;
	return m_Output.Add(wxT("organindex.ini"), buf);
}

bool GOrgueArchiveCreator::FinishPackage()
{
	for(unsigned i = 0; i < m_organs.size(); i++)
	{
		if (m_organs[i])
			continue;
		std::unique_ptr<GOrgueFile> f = findPackageFile(m_OrganPaths[i]);
		if (!f)
		{
			wxLogError(_("organ definition %s not found in organ package"), m_OrganPaths[i].c_str());
			return false;
		}
		if (!addOrganData(i, f.get()))
			return false;
	}
	if (!writePackageIndex())
	{
		wxLogError(_("Failed writing the package index file"));
		return false;
	}
	if (!m_Output.Close())
	{
		wxLogError(_("Failed writing the package directory"));
		return false;
	}
	return true;
}

bool GOrgueArchiveCreator::checkExtension(const wxString& name, wxString ext)
{
	if (ext != ext.Lower())
	{
		wxLogWarning(_("%s: extension with uppercase letters"), name.c_str());
		ext = ext.Lower();
	}
	if (ext == wxT("wav") || ext == wxT("wv"))
		return true;
	if (ext == wxT("png") || ext == wxT("gif") || ext == wxT("jpg") || ext == wxT("bmp"))
		return true;
	if (ext == wxT("organ"))
		return true;
	return false;
}

bool GOrgueArchiveCreator::AddDirectory(const wxString& path)
{
	wxString dir = GONormalizePath(path);
	if (!wxDirExists(dir))
	{
		wxLogError(_("Input directory %s not found"), path.c_str());
		return false;
	}
	wxArrayString files;
	wxDir::GetAllFiles(dir, &files);
	files.Sort();
	for(unsigned i = 0; i < files.size(); i++)
	{
		wxFileName fname(files[i]);
		GOrgueBuffer<uint8_t> buf;
		GOrgueStandardFile f(files[i]);
		if (!f.ReadContent(buf))
		{
			wxLogError(_("Failed to read file %s"), files[i].c_str());
			return false;
		}
		if (!checkExtension(files[i], fname.GetExt()))
			wxLogError(_("Unknown filetype %s"), files[i].c_str());
		if (!fname.MakeRelativeTo(dir))
		{
			wxLogError(_("failed to create relative path"));
			return false;
		}
		wxString fn = fname.GetLongPath();
		fn.Replace(wxFileName::GetPathSeparator(), wxT("\\"));
		if (fn == "organindex.ini")
			continue;
		if (!compressData(fn, fname.GetExt().Lower(), buf))
		{
			wxLogError(_("failed to compress data %s"), fn.c_str());
			return false;
		}
		for(unsigned i = 0; i < m_OrganPaths.size(); i++)
			if (m_OrganPaths[i] == fn)
				if (!addOrganData(i, &f))
					return false;
		if (!storeFile(fn, buf))
			return false;
	}
	return true;
}

bool GOrgueArchiveCreator::addOrganData(unsigned idx, GOrgueFile* file)
{
	GOrgueConfigFileReader cfg;
	if (!cfg.Read(file))
	{
		wxLogError(_("Failed to parse organ definition %s"), m_OrganPaths[idx].c_str());
		return false;
	}
	std::map<wxString, std::map<wxString, wxString> >::const_iterator organ_section = cfg.GetContent().find(wxT("Organ"));
	if (organ_section == cfg.GetContent().end())
	{
		wxLogError(_("No organ section in organ definition %s"), m_OrganPaths[idx].c_str());
		return false;
	}
	const std::map<wxString, wxString> map = organ_section->second;
	std::map<wxString, wxString>::const_iterator it;
	wxString church_name, organ_builder, recording_details;
	it = map.find(wxT("ChurchName"));
	if (it != map.end())
		church_name = it->second;
	else
	{
		wxLogError(_("ChurchName missing in organ definition %s"), m_OrganPaths[idx].c_str());
		return false;
	}
	it = map.find(wxT("OrganBuilder"));
	if (it != map.end())
		organ_builder = it->second;
	it = map.find(wxT("RecordingDetails"));
	if (it != map.end())
		recording_details = it->second;
	m_organs[idx] = new GOrgueOrgan(m_OrganPaths[idx], wxEmptyString, church_name, organ_builder, recording_details);
	return true;
}

bool GOrgueArchiveCreator::compressData(const wxString& name, const wxString& ext, GOrgueBuffer<uint8_t>& data)
{
	if (GOrgueWave::IsWaveFile(data))
	{
		GOrgueWave wav;
		try
		{
			wav.Open(data);
		}
		catch(wxString error)
		{
			wxLogError(_("Failed to read wav file %s: %s"), name.c_str(), error.c_str());
			return false;
		}
		return wav.Save(data);
	}
	if (ext == wxT("organ"))
	{
		if (!isBufferCompressed(data))
			return compressBuffer(data);
	}
	return true;
}
