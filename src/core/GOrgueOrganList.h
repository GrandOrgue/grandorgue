/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
