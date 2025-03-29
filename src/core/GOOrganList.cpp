/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganList.h"

#include <algorithm>

#include <wx/dir.h>
#include <wx/stdpaths.h>

GOOrgan *GOOrganList::CloneOrgan(const GOOrgan &newOrgan) const {
  return new GOOrgan(newOrgan);
}

void GOOrganList::RemoveInvalidTmpOrgans() {
  const wxString tmpDirPrefix
    = wxDir(wxStandardPaths::Get().GetTempDir()).GetNameWithSep();

  for (int i = m_OrganList.size() - 1; i >= 0; i--) {
    const GOOrgan &o = *m_OrganList[i];

    if (!o.IsUsable(*this) && o.GetPath().StartsWith(tmpDirPrefix))
      m_OrganList.erase(i);
  }
  for (int i = m_ArchiveList.size() - 1; i >= 0; i--) {
    const GOArchiveFile &aF = *m_ArchiveList[i];
    const wxString &archivePath = aF.GetPath();

    if (
      !archivePath.IsEmpty() && !aF.IsUsable()
      && archivePath.StartsWith(tmpDirPrefix))
      m_ArchiveList.erase(i);
  }
}

static bool LRUCompare(const GOOrgan *a, const GOOrgan *b) {
  return a->GetLastUse() > b->GetLastUse();
}

std::vector<const GOOrgan *> GOOrganList::GetLRUOrganList() {
  std::vector<const GOOrgan *> lru;
  for (unsigned i = 0; i < m_OrganList.size(); i++)
    if (m_OrganList[i]->IsUsable(*this))
      lru.push_back(m_OrganList[i]);
  std::sort(lru.begin(), lru.end(), LRUCompare);
  return lru;
}

void GOOrganList::AddOrgan(const GOOrgan &newOrgan) {
  for (GOOrgan *pExOrgan : m_OrganList)
    if (
      pExOrgan->GetODFPath() == newOrgan.GetODFPath()
      && pExOrgan->GetArchiveID() == newOrgan.GetArchiveID()
      && (
	pExOrgan->GetArchivePath().IsEmpty() 
	|| pExOrgan->GetArchivePath() == newOrgan.GetArchivePath()
      )
    ) {
      pExOrgan->Update(newOrgan);
      return;
    }
  AddNewOrgan(CloneOrgan(newOrgan));
}

const GOArchiveFile *GOOrganList::GetArchiveByID(
  const wxString &id, bool useable) const {
  for (unsigned i = 0; i < m_ArchiveList.size(); i++)
    if (m_ArchiveList[i]->GetID() == id)
      if (!useable || m_ArchiveList[i]->IsUsable())
        return m_ArchiveList[i];
  return NULL;
}

const GOArchiveFile *GOOrganList::GetArchiveByPath(const wxString &path) const {
  for (unsigned i = 0; i < m_ArchiveList.size(); i++)
    if (m_ArchiveList[i]->GetPath() == path)
      return m_ArchiveList[i];
  return NULL;
}

void GOOrganList::AddArchive(const GOArchiveFile &archive) {
  for (unsigned i = 0; i < m_ArchiveList.size(); i++)
    if (m_ArchiveList[i]->GetPath() == archive.GetPath()) {
      m_ArchiveList[i]->Update(archive);
      return;
    }
  AddNewArchive(new GOArchiveFile(archive));
}

void GOOrganList::AddOrgansFromArchives() {
  for (const GOArchiveFile *pA : m_ArchiveList) {
    const wxString &archivePath = pA->GetPath();
    const wxString &archiveId = pA->GetID();

    GOOrgan *pOMatchedByPath = NULL;
    GOOrgan *pOMatchedById = NULL;

    for (GOOrgan *pO : m_OrganList) {
      const wxString &organArchivePath = pO->GetArchivePath();

      if (organArchivePath == archivePath) {
        pOMatchedByPath = pO;
        break;
      }
      if (
        !pOMatchedById && organArchivePath.IsEmpty()
        && pO->GetArchiveID() == archiveId)
        pOMatchedById = pO;
    }
    if (!pOMatchedByPath) {
      if (pOMatchedById)
        pOMatchedById->SetArchivePath(archivePath);
    }
  }
}
