/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEARCHIVEFILE_H
#define GORGUEARCHIVEFILE_H

#include <wx/string.h>
#include <vector>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueSettings;

class GOrgueArchiveFile
{
private:
	wxString m_ID;
	wxString m_FileID;
	wxString m_Path;
	wxString m_Name;
	std::vector<wxString> m_Dependencies;
	std::vector<wxString> m_DependencyTitles;

public:
	GOrgueArchiveFile(wxString id, wxString path, wxString name, const std::vector<wxString>& dependencies, const std::vector<wxString>& dependency_titles);
	GOrgueArchiveFile(GOrgueConfigReader& cfg, wxString group);
	virtual ~GOrgueArchiveFile();

	void Update(const GOrgueArchiveFile& archive);
	wxString GetCurrentFileID();

	void Save(GOrgueConfigWriter& cfg, wxString group);

	const wxString& GetID() const;
	const wxString& GetPath() const;
	const wxString& GetName() const;
	const wxString& GetFileID() const;
	const std::vector<wxString>& GetDependencies() const;
	const std::vector<wxString>& GetDependencyTitles() const;

	bool IsUsable(GOrgueSettings& settings);
	bool IsComplete(GOrgueSettings& settings);
};

#endif
