/*
 * GrandOrgue - free pipe organ simulator
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

#include "GOrgueFilename.h"

#include "GOrgueArchive.h"
#include "GOrgueHash.h"
#include "GOrgueInvalidFile.h"
#include "GOrgueSettings.h"
#include "GOrgueStandardFile.h"
#include "GrandOrgueFile.h"
#include <wx/filename.h>
#include <wx/log.h>

GOrgueFilename::GOrgueFilename() :
	m_Name(),
	m_Path(),
	m_Archiv(NULL),
	m_Hash(true)
{
}

const wxString& GOrgueFilename::GetTitle() const
{
	return m_Name;
}

void GOrgueFilename::Hash(GOrgueHash& hash) const
{
	if (m_Archiv)
	{
		hash.Update(m_Archiv->GetArchiveID());
		hash.Update(m_Name);
		return;
	}
	hash.Update(m_Path);
	wxFileName path_name(m_Path);
	if (!m_Hash)
		return;
	uint64_t size = path_name.GetSize().GetValue();
	uint64_t time = path_name.GetModificationTime().GetTicks();
	hash.Update(time);
	hash.Update(size);
}

std::unique_ptr<GOrgueFile> GOrgueFilename::Open() const
{
	if (m_Archiv)
		return (std::unique_ptr<GOrgueFile>)m_Archiv->OpenFile(m_Name);

	if (m_Path != wxEmptyString && wxFileExists(m_Path))
		return (std::unique_ptr<GOrgueFile>)new GOrgueStandardFile(m_Path, m_Name);
	else
		return (std::unique_ptr<GOrgueFile>)new GOrgueInvalidFile(m_Name);
}

void GOrgueFilename::SetPath(const wxString& base, const wxString& file)
{
	wxString temp = file;
	temp.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));
	temp = base + wxFileName::GetPathSeparator() + temp;

	wxFileName path_name(temp);
	path_name.Normalize(wxPATH_NORM_DOTS);
	if (path_name.GetLongPath() != path_name.GetFullPath())
	{
		wxLogWarning(_("Filename '%s' not compatible with case sensitive systems"), file.c_str());
	}
	if (!wxFileExists(temp))
	{
		wxLogError(_("File '%s' does not exists"), file.c_str());
		m_Hash = false;
	}
	m_Path = temp;
}

void GOrgueFilename::Assign(const wxString& name, GrandOrgueFile* organfile)
{
	m_Name = name;
	if (organfile->GetSettings().ODFCheck() && name.Find(wxT('/')) != wxNOT_FOUND)
	{
		wxLogWarning(_("Filename '%s' contains non-portable directory seperator /"), name.c_str());
	}
	if (organfile->useArchives())
	{
		m_Path = wxEmptyString;
		m_Archiv = organfile->findArchive(name);
		if (!m_Archiv)
		{
			wxLogError(_("File '%s' does not exists"), name.c_str());
		}
		return;
	}
	SetPath(organfile->GetODFPath(), name);
}

void GOrgueFilename::AssignResource(const wxString& name, GrandOrgueFile* organfile)
{
	m_Name = name;
	m_Hash = false;
	SetPath(organfile->GetSettings().GetResourceDirectory(), name);
}

void GOrgueFilename::AssignAbsolute(const wxString& path)
{
	m_Name = path;
	m_Path = path;
	m_Hash = false;
}
