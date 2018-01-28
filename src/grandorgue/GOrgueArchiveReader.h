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

#ifndef GORGUEARCHIVEREADER_H
#define GORGUEARCHIVEREADER_H

#include "GOrgueZipFormat.h"
#include <wx/string.h>
#include <vector>

class wxFile;
typedef struct _GOArchiveEntry GOArchiveEntry;

class GOrgueArchiveReader
{
private:
	wxFile& m_File;

	bool Seek(size_t offset);
	bool Read(void* buf, size_t len);
	bool GenerateFileHash(wxString& id);
	size_t ExtractU64(void* ptr);
	size_t ExtractU32(void* ptr);

	bool ReadFileRecord(size_t central_offset, GOZipCentralHeader& central, std::vector<GOArchiveEntry>& entries);
	bool ReadCentralDirectory(size_t offset, size_t entry_count, size_t length, std::vector<GOArchiveEntry>& entries);
	bool ReadEnd64Record(size_t offset, GOZipEnd64Record& record);
	bool ReadEndRecord(std::vector<GOArchiveEntry>& entries);

public:
	GOrgueArchiveReader(wxFile& file);
	~GOrgueArchiveReader();

	bool ListContent(wxString& id, std::vector<GOArchiveEntry>& entries);
};

#endif
