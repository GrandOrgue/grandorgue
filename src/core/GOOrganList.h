/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANLIST_H
#define GOORGANLIST_H

#include <wx/string.h>

#include "ptrvector.h"

#include "archive/GOArchiveFile.h"

#include "GOOrgan.h"

class GOOrganList {
private:
  ptr_vector<GOOrgan> m_OrganList;
  ptr_vector<GOArchiveFile> m_ArchiveList;

protected:
  virtual GOOrgan *CloneOrgan(const GOOrgan &newOrgan) const;
  virtual bool IsValidOrgan(const GOOrgan *pOrgan) const {
    return pOrgan != nullptr;
  }
  void AddNewOrgan(GOOrgan *pOrgan);
  void AddNewArchive(GOArchiveFile *pArchive) {
    m_ArchiveList.push_back(pArchive);
  }

public:
  void ClearOrgans() { m_OrganList.clear(); }
  void AddOrgan(const GOOrgan &organ);
  void RemoveInvalidTmpOrgans();
  const ptr_vector<GOOrgan> &GetOrganList() const { return m_OrganList; }
  ptr_vector<GOOrgan> &GetOrganList() { return m_OrganList; }
  std::vector<const GOOrgan *> GetLRUOrganList();

  void ClearArchives() { m_ArchiveList.clear(); }
  void AddArchive(const GOArchiveFile &archive);
  ptr_vector<GOArchiveFile> &GetArchiveList() { return m_ArchiveList; }
  const ptr_vector<GOArchiveFile> &GetArchiveList() const {
    return m_ArchiveList;
  }
  const GOArchiveFile *GetArchiveByID(
    const wxString &id, bool useable = false) const;
  const GOArchiveFile *GetArchiveByPath(const wxString &path) const;

  void AddOrgansFromArchives();
};

#endif
