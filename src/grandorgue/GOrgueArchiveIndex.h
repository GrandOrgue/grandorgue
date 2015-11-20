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

#ifndef GORGUEARCHIVEINDEX_H
#define GORGUEARCHIVEINDEX_H

#include <wx/file.h>
#include <wx/string.h>
#include <vector>

class GOrgueSettings;
typedef struct _GOrgueHashType GOrgueHashType;

typedef struct _GOArchiveEntry
{
	wxString name;
	size_t offset;
	size_t len;
} GOArchiveEntry;

class GOrgueArchiveIndex
{
private:
	GOrgueSettings& m_Settings;
	wxString m_Path;
	wxFile m_File;

	GOrgueHashType GenerateHash();
	wxString GenerateIndexFilename();

	bool Write(const void* buf, unsigned len);
	bool Read(void* buf, unsigned len);

	bool WriteString(const wxString& str);
	bool ReadString(wxString& str);

	bool WriteEntry(const GOArchiveEntry& e);
	bool ReadEntry(GOArchiveEntry& e);

	bool ReadContent(wxString& id, std::vector<GOArchiveEntry>& entries);
	bool WriteContent(const wxString& id, const std::vector<GOArchiveEntry>& entries);

public:
	GOrgueArchiveIndex(GOrgueSettings& settings, const wxString& path);
	~GOrgueArchiveIndex();

	bool ReadIndex(wxString& id, std::vector<GOArchiveEntry>& entries);
	bool WriteIndex(const wxString& id, const std::vector<GOArchiveEntry>& entries);
};

#endif
