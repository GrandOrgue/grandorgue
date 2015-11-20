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

#include "GOrgueArchiveIndex.h"

#include "GOrgueHash.h"
#include "GOrgueSettings.h"
#include <wx/filename.h>
#include <wx/log.h>

/* Value which is used to identify a valid cache index file. */
#define GRANDORGUE_INDEX_MAGIC 0x43214321

GOrgueArchiveIndex::GOrgueArchiveIndex(GOrgueSettings& settings, const wxString& path) :
	m_Settings(settings),
	m_Path(path),
	m_File()
{
}

GOrgueArchiveIndex::~GOrgueArchiveIndex()
{
	m_File.Close();
}

wxString GOrgueArchiveIndex::GenerateIndexFilename()
{
	GOrgueHash hash;
	hash.Update(m_Path);

	return m_Settings.UserCachePath()  + wxFileName::GetPathSeparator() + 
		hash.getStringHash() + wxT(".idx");
}

GOrgueHashType GOrgueArchiveIndex::GenerateHash()
{
	GOrgueHash hash;
	hash.Update(sizeof(wxString));
	hash.Update(sizeof(GOArchiveEntry));
	hash.Update(sizeof(size_t));

	wxFileName path_name(m_Path);
	uint64_t size = path_name.GetSize().GetValue();
	uint64_t time = path_name.GetModificationTime().GetTicks();
	hash.Update(time);
	hash.Update(size);

	return hash.getHash();
}

bool GOrgueArchiveIndex::Write(const void* buf, unsigned len)
{
	return m_File.Write(buf, len) == len;
}

bool GOrgueArchiveIndex::Read(void* buf, unsigned len)
{
	return m_File.Read(buf, len) == len;
}

bool GOrgueArchiveIndex::WriteString(const wxString& str)
{
	size_t len = str.Length();
	if (!Write(&len, sizeof(len)))
		return false;
	if (!Write((const wxChar*)str.c_str(), len * sizeof(wxChar)))
		return false;
	return true;
}

bool GOrgueArchiveIndex::ReadString(wxString& str)
{
	size_t len;
	if (!Read(&len, sizeof(len)))
		return false;
	wxWCharBuffer buffer(len);
	if (!Read(buffer.data(), len * sizeof(wxChar)))
		return false;
	str = buffer;
	return true;
}

bool GOrgueArchiveIndex::WriteEntry(const GOArchiveEntry& e)
{
	if (!WriteString(e.name))
		return false;
	if (!Write(&e.offset, sizeof(e.offset)))
		return false;
	if (!Write(&e.len, sizeof(e.len)))
		return false;
	return true;
}

bool GOrgueArchiveIndex::ReadEntry(GOArchiveEntry& e)
{
	if (!ReadString(e.name))
		return false;
	if (!Read(&e.offset, sizeof(e.offset)))
		return false;
	if (!Read(&e.len, sizeof(e.len)))
		return false;
	return true;
}

bool GOrgueArchiveIndex::ReadContent(wxString& id, std::vector<GOArchiveEntry>& entries)
{
	if (!ReadString(id))
		return false;

	unsigned cnt;
	if (!Read(&cnt, sizeof(cnt)))
	    return false;
	entries.resize(cnt);
	for(unsigned i = 0; i < entries.size(); i++)
		if (!ReadEntry(entries[i]))
			return false;

	return true;
}

bool GOrgueArchiveIndex::WriteContent(const wxString& id, const std::vector<GOArchiveEntry>& entries)
{
	int magic = GRANDORGUE_INDEX_MAGIC;
	if (!Write(&magic, sizeof(magic)))
		return false;

	GOrgueHashType hash = GenerateHash();
	if (!Write(&hash, sizeof(hash)))
		return false;

	if (!WriteString(id))
		return false;

	unsigned cnt = entries.size();
	if (!Write(&cnt, sizeof(cnt)))
	    return false;
	for(unsigned i = 0; i < entries.size(); i++)
		if (!WriteEntry(entries[i]))
			return false;

	return true;
}

bool GOrgueArchiveIndex::ReadIndex(wxString& id, std::vector<GOArchiveEntry>& entries)
{
	wxString name = GenerateIndexFilename();
	if (!wxFileExists(name))
		return false;
	if (!m_File.Open(name, wxFile::read))
	{
		wxLogError(_("Failed to open '%s'"), name.c_str());
		return false;
	}

	int magic;
	GOrgueHashType hash1, hash2;
	hash1 = GenerateHash();

	if (!Read(&magic, sizeof(magic)) || !Read(&hash2, sizeof(hash2)))
	{
		m_File.Close();
		wxLogError(_("Failed to read '%s'"), name.c_str());
		return false;
	}
	if (magic != GRANDORGUE_INDEX_MAGIC || memcmp(&hash1, &hash2, sizeof(hash1)))
	{
		m_File.Close();
		wxLogError(_("Index '%s' has bad magic - bypassing index"), name.c_str());
		return false;
	}

	if (!ReadContent(id, entries))
	{
		m_File.Close();
		wxLogError(_("Failed to read '%s'"), name.c_str());
		return false;
	}

	m_File.Close();
	return true;
}

bool GOrgueArchiveIndex::WriteIndex(const wxString& id, const std::vector<GOArchiveEntry>& entries)
{
	wxString name = GenerateIndexFilename();
	if (!m_File.Create(name, true) || !m_File.IsOpened())
	{
		m_File.Close();
		wxLogError(_("Failed to write to '%s'"), name.c_str());
		return false;
	}

	if (!WriteContent(id, entries))
	{
		m_File.Close();
		wxLogError(_("Failed to write content to '%s'"), name.c_str());
		return false;
	}

	m_File.Close();
	return true;
}
