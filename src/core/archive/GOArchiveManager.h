/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOARCHIVEMANAGER_H
#define GOARCHIVEMANAGER_H

#include <wx/string.h>

class GOArchive;
class GOOrganList;

class GOArchiveManager {
private:
  GOOrganList &m_OrganList;
  wxString m_CacheDir;

  GOArchive *OpenArchive(const wxString &path);
  bool ReadIndex(GOArchive *archive, bool InstallOrgans = false);
  wxString InstallPackage(const wxString &path, const wxString &last_id);

public:
  GOArchiveManager(GOOrganList &OrganList, const wxString &cacheDir);
  ~GOArchiveManager();

  GOArchive *LoadArchive(const wxString &id, const wxString &archivePath);
  wxString InstallPackage(const wxString &path);
  bool RegisterPackage(const wxString &path);
  void RegisterPackageDirectory(const wxString &path);
};

#endif
