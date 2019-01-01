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

#include "GOrgueArchive.h"

#include "GOrgueArchiveEntryFile.h"
#include "GOrgueArchiveIndex.h"
#include "GOrgueArchiveReader.h"
#include "GOrgueInvalidFile.h"
#include "mutex_locker.h"
#include <wx/intl.h>
#include <wx/log.h>

GOrgueArchive::GOrgueArchive(const GOrgueSettingDirectory& cachePath) :
	m_CachePath(cachePath),
	m_ID(),
	m_Dependencies(),
	m_Entries(),
	m_Path()
{
}

GOrgueArchive::~GOrgueArchive()
{
	Close();
}

bool GOrgueArchive::OpenArchive(const wxString& path)
{
	m_Entries.clear();
	m_Path = path;
	if (!m_File.Open(path, wxFile::read))
	{
		wxLogError(_("Failed to open '%s'"), path.c_str());
		return false;
	}
	{
		GOrgueArchiveIndex index(m_CachePath, m_Path);
		if (index.ReadIndex(m_ID, m_Entries))
			return true;
	}

	GOrgueArchiveReader reader(m_File);
	if (!reader.ListContent(m_ID, m_Entries))
	{
		wxLogError(_("Failed to parse '%s'"), path.c_str());
		return false;
	}

	GOrgueArchiveIndex index(m_CachePath, m_Path);
	index.WriteIndex(m_ID, m_Entries);
	return true;
}

void GOrgueArchive::Close()
{
	m_File.Close();
	m_Entries.clear();
}

bool GOrgueArchive::containsFile(const wxString& name)
{
	for(unsigned i = 0; i < m_Entries.size(); i++)
		if (m_Entries[i].name == name)
			return true;
	return false;
}

GOrgueFile* GOrgueArchive::OpenFile(const wxString& name)
{
	for(unsigned i = 0; i < m_Entries.size(); i++)
		if (m_Entries[i].name == name)
		{
			return new GOrgueArchiveEntryFile(this, m_Entries[i].name, m_Entries[i].offset, m_Entries[i].len);
		}
	return new GOrgueInvalidFile(name);
}

size_t GOrgueArchive::ReadContent(void* buffer, size_t offset, size_t len)
{
	GOMutexLocker lock(m_Mutex);
	ssize_t pos = m_File.Seek(offset);
	if (pos != (ssize_t)offset)
		return 0;
	ssize_t l = m_File.Read(buffer, len);
	if (l == wxInvalidOffset)
		return 0;
	return l;
}

const wxString& GOrgueArchive::GetArchiveID()
{
	return m_ID;
}
const wxString& GOrgueArchive::GetPath()
{
	return m_Path;
}

const std::vector<wxString>& GOrgueArchive::GetDependencies() const
{
	return m_Dependencies;
}

void GOrgueArchive::SetDependencies(const std::vector<wxString>& dependencies)
{
	m_Dependencies = dependencies;
}
