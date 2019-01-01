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

#ifndef GORGUEARCHIVEWRITER_H
#define GORGUEARCHIVEWRITER_H

#include "GOrgueBuffer.h"
#include "GOrgueZipFormat.h"
#include <wx/file.h>
#include <vector>

class GOrgueArchiveWriter
{
private:
	wxFile m_File;
	size_t m_Offset;
	GOrgueBuffer<uint8_t> m_directory;
	unsigned m_Entries;
	std::vector<wxString> m_Names;

	bool Write(const void* data, size_t size);

public:
	GOrgueArchiveWriter();
	~GOrgueArchiveWriter();

	bool Open(wxString filename);
	bool Add(wxString name, const GOrgueBuffer<uint8_t>& content);
	bool Close();
};

#endif
