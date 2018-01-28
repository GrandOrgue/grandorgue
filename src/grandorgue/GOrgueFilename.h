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

#ifndef GORGUEFILENAME_H
#define GORGUEFILENAME_H

#include <wx/string.h>
#include <memory>

class GOrgueArchive;
class GOrgueFile;
class GOrgueHash;
class GrandOrgueFile;

class GOrgueFilename
{
private:
	wxString m_Name;
	wxString m_Path;
	GOrgueArchive* m_Archiv;
	bool m_Hash;

	void SetPath(const wxString& base, const wxString& path);
	
public:
	GOrgueFilename();

	void Assign(const wxString& name, GrandOrgueFile* organfile);
	void AssignResource(const wxString& name, GrandOrgueFile* organfile);
	void AssignAbsolute(const wxString& path);

	const wxString& GetTitle() const;
	void Hash(GOrgueHash& hash) const;

	std::unique_ptr<GOrgueFile> Open() const;
};

#endif
