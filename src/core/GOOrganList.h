/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANLIST_H
#define GOORGANLIST_H

#include <wx/string.h>

#include "ptrvector.h"

class GOArchiveFile;
class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;
class GOOrgan;

class GOOrganList {
private:
  ptr_vector<GOOrgan> m_OrganList;
  ptr_vector<GOArchiveFile> m_ArchiveList;

protected:
  void Load(GOConfigReader &cfg, GOMidiMap &map);
  void Save(GOConfigWriter &cfg, GOMidiMap &map);

public:
  GOOrganList();
  ~GOOrganList();

  void AddOrgan(const GOOrgan &organ);
  void RemoveInvalidTmpOrgans();
  const ptr_vector<GOOrgan> &GetOrganList() const { return m_OrganList; }
  ptr_vector<GOOrgan> &GetOrganList() { return m_OrganList; }
  std::vector<const GOOrgan *> GetLRUOrganList();

  void AddArchive(const GOArchiveFile &archive);
  ptr_vector<GOArchiveFile> &GetArchiveList();
  const ptr_vector<GOArchiveFile> &GetArchiveList() const;
  const GOArchiveFile *GetArchiveByID(
    const wxString &id, bool useable = false) const;
  const GOArchiveFile *GetArchiveByPath(const wxString &path) const;

  void AddOrgansFromArchives();
};

#endif
