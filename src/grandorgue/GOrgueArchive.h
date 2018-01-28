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

#ifndef GORGUEARCHIVE_H
#define GORGUEARCHIVE_H

#include "mutex.h"
#include <wx/file.h>
#include <wx/string.h>
#include <vector>

class GOrgueFile;
class GOrgueSettings;
typedef struct _GOArchiveEntry GOArchiveEntry;

class GOrgueArchive
{
private:
	GOMutex m_Mutex;
	GOrgueSettings& m_Settings;
	wxString m_ID;
	std::vector<wxString> m_Dependencies;
	std::vector<GOArchiveEntry> m_Entries;
	wxFile m_File;
	wxString m_Path;

public:
	GOrgueArchive(GOrgueSettings& settings);
	~GOrgueArchive();

	bool OpenArchive(const wxString& path);
	void Close();

	bool containsFile(const wxString& name);
	GOrgueFile* OpenFile(const wxString& name);

	size_t ReadContent(void* buffer, size_t offset, size_t len);

	const wxString& GetArchiveID();
	const wxString& GetPath();

	const std::vector<wxString>& GetDependencies() const;
	void SetDependencies(const std::vector<wxString>& dependencies);
};

#endif
