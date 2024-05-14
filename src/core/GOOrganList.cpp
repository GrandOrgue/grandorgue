/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganList.h"

#include <algorithm>

#include <wx/dir.h>
#include <wx/stdpaths.h>

#include "archive/GOArchiveFile.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

#include "GOOrgan.h"

GOOrganList::GOOrganList() : m_OrganList(), m_ArchiveList() {}

GOOrganList::~GOOrganList() {}

void GOOrganList::Load(GOConfigReader &cfg, GOMidiMap &map) {
  m_OrganList.clear();
  unsigned organ_count = cfg.ReadInteger(
    CMBSetting, wxT("General"), wxT("OrganCount"), 0, 99999, false, 0);
  for (unsigned i = 0; i < organ_count; i++)
    m_OrganList.push_back(
      new GOOrgan(cfg, wxString::Format(wxT("Organ%03d"), i + 1), map));

  m_ArchiveList.clear();
  unsigned archive_count = cfg.ReadInteger(
    CMBSetting, wxT("General"), wxT("ArchiveCount"), 0, 99999, false, 0);
  for (unsigned i = 0; i < archive_count; i++)
    m_ArchiveList.push_back(
      new GOArchiveFile(cfg, wxString::Format(wxT("Archive%03d"), i + 1)));
}

void GOOrganList::Save(GOConfigWriter &cfg, GOMidiMap &map) {
  cfg.WriteInteger(wxT("General"), wxT("ArchiveCount"), m_ArchiveList.size());
  for (unsigned i = 0; i < m_ArchiveList.size(); i++)
    m_ArchiveList[i]->Save(cfg, wxString::Format(wxT("Archive%03d"), i + 1));

  cfg.WriteInteger(wxT("General"), wxT("OrganCount"), m_OrganList.size());
  for (unsigned i = 0; i < m_OrganList.size(); i++)
    m_OrganList[i]->Save(cfg, wxString::Format(wxT("Organ%03d"), i + 1), map);
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
  m_OrganList.push_back(new GOOrgan(newOrgan));
}

const ptr_vector<GOArchiveFile> &GOOrganList::GetArchiveList() const {
  return m_ArchiveList;
}

ptr_vector<GOArchiveFile> &GOOrganList::GetArchiveList() {
  return m_ArchiveList;
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
  m_ArchiveList.push_back(new GOArchiveFile(archive));
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
