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

#ifndef GORGUEORGANLIST_H
#define GORGUEORGANLIST_H

#include "ptrvector.h"
#include <wx/string.h>

class GOrgueArchiveFile;
class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueMidiMap;
class GOrgueOrgan;

class GOrgueOrganList
{
private:
	ptr_vector<GOrgueOrgan> m_OrganList;
	ptr_vector<GOrgueArchiveFile> m_ArchiveList;

protected:
	void Load(GOrgueConfigReader& cfg, GOrgueMidiMap& map);
	void Save(GOrgueConfigWriter& cfg, GOrgueMidiMap& map);

public:
	GOrgueOrganList();
	~GOrgueOrganList();

	void AddOrgan(const GOrgueOrgan& organ);
	const ptr_vector<GOrgueOrgan>& GetOrganList() const;
	ptr_vector<GOrgueOrgan>& GetOrganList();
	std::vector<const GOrgueOrgan*> GetLRUOrganList();

	void AddArchive(const GOrgueArchiveFile& archive);
	ptr_vector<GOrgueArchiveFile>& GetArchiveList();
	const ptr_vector<GOrgueArchiveFile>& GetArchiveList() const;
	const GOrgueArchiveFile* GetArchiveByID(const wxString& id, bool useable = false) const;
	const GOrgueArchiveFile* GetArchiveByPath(const wxString& path) const;
};

#endif
