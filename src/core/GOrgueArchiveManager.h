/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEARCHIVEMANAGER_H
#define GORGUEARCHIVEMANAGER_H

class GOrgueArchive;
class GOrgueOrganList;
class GOrgueSettings;
class GOrgueSettingDirectory;

#include <wx/string.h>

class GOrgueArchiveManager
{
private:
	GOrgueOrganList& m_OrganList;
	const GOrgueSettingDirectory& m_CacheDir;

	GOrgueArchive* OpenArchive(const wxString& path);
	bool ReadIndex(GOrgueArchive* archive, bool InstallOrgans = false);
	wxString InstallPackage(const wxString& path, const wxString& last_id);

public:
	GOrgueArchiveManager(GOrgueOrganList& OrganList, const GOrgueSettingDirectory& CacheDir);
	~GOrgueArchiveManager();

	GOrgueArchive* LoadArchive(const wxString& id);
	wxString InstallPackage(const wxString& path);
	bool RegisterPackage(const wxString& path);
	void RegisterPackageDirectory(const wxString& path);
};

#endif
