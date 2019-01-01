/*
 * GrandOrgue - a free pipe organ simulator
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

#ifndef GRANDORGUEARCHIVECREATOR_H
#define GRANDORGUEARCHIVECREATOR_H

#include "GOrgueArchiveManager.h"
#include "GOrgueArchiveWriter.h"
#include "GOrgueOrganList.h"

class GOrgueFile;
class GOrgueOrgan;

class GOrgueArchiveCreator
{
private:
	GOrgueOrganList m_OrganList;
	GOrgueArchiveManager m_Manager;
	GOrgueArchiveWriter m_Output;
	std::vector<const GOrgueArchiveFile*> m_packageIDs;
	ptr_vector<GOrgueArchive> m_packages;
	ptr_vector<GOrgueOrgan> m_organs;
	std::vector<wxString> m_OrganPaths;
	wxString m_PackageTitle;

	std::unique_ptr<GOrgueFile> findPackageFile(const wxString& name);
	bool writePackageIndex();
	bool checkExtension(const wxString& name, wxString ext);
	bool storeFile(const wxString& name, const GOrgueBuffer<uint8_t>& data);
	bool addOrganData(unsigned idx, GOrgueFile* file);
	bool compressData(const wxString& name, const wxString& ext, GOrgueBuffer<uint8_t>& data);

public:
	GOrgueArchiveCreator(const GOrgueSettingDirectory& cacheDir);
	~GOrgueArchiveCreator();

	bool CreatePackage(const wxString& path, const wxString title);
	bool AddPackage(const wxString& path);
	void AddOrgan(const wxString& path);
	bool AddDirectory(const wxString& path);
	bool FinishPackage();
};

#endif
