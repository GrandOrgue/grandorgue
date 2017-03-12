/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUESTANDARDFILE_H
#define GORGUESTANDARDFILE_H

#include "GOrgueFile.h"
#include <wx/file.h>

class GOrgueStandardFile : public GOrgueFile
{
private:
	wxString m_Path;
	wxString m_Name;
	wxFile m_File;
	size_t m_Size;

public:
	GOrgueStandardFile(const wxString& path);
	GOrgueStandardFile(const wxString& path, const wxString& name);
	~GOrgueStandardFile();

	size_t GetSize();
	const wxString GetName();
	const wxString GetPath();

	bool Open();
	void Close();
	size_t Read(void * buffer, size_t len);
};

#endif
