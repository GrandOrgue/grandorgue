/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOArchiveFile.h"

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>

#include "GOHash.h"
#include "GOOrganList.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOArchiveFile::GOArchiveFile(
  wxString id,
  wxString path,
  wxString name,
  const std::vector<wxString> &dependencies,
  const std::vector<wxString> &dependency_titles)
  : m_ID(id),
    m_Path(path),
    m_Name(name),
    m_Dependencies(dependencies),
    m_DependencyTitles(dependency_titles) {
  m_FileID = GetCurrentFileID();
}

GOArchiveFile::GOArchiveFile(GOConfigReader &cfg, wxString group) {
  m_ID = cfg.ReadString(CMBSetting, group, wxT("ID"));
  m_Path = cfg.ReadString(CMBSetting, group, wxT("Path"));
  m_Name = cfg.ReadString(CMBSetting, group, wxT("Name"));
  m_Dependencies.resize(
    cfg.ReadInteger(CMBSetting, group, wxT("DependenciesCount"), 0, 999));
  m_FileID = cfg.ReadString(CMBSetting, group, wxT("FileID"));
  for (unsigned i = 0; i < m_Dependencies.size(); i++) {
    m_Dependencies[i] = cfg.ReadString(
      CMBSetting, group, wxString::Format("Dependency%03d", i + 1));
    m_DependencyTitles[i] = cfg.ReadString(
      CMBSetting, group, wxString::Format("DependencyTitle%03d", i + 1));
  }
}

GOArchiveFile::~GOArchiveFile() {}

wxString GOArchiveFile::getArchiveHash(const wxString &path) {
  GOHash hash;

  hash.Update(path);
  return hash.getStringHash();
}

void GOArchiveFile::Save(GOConfigWriter &cfg, wxString group) {
  cfg.WriteString(group, wxT("ID"), m_ID);
  cfg.WriteString(group, wxT("Path"), m_Path);
  cfg.WriteString(group, wxT("Name"), m_Name);
  cfg.WriteString(group, wxT("FileID"), m_FileID);
  cfg.WriteInteger(group, wxT("DependenciesCount"), m_Dependencies.size());
  for (unsigned i = 0; i < m_Dependencies.size(); i++) {
    cfg.WriteString(
      group, wxString::Format("Dependency%03d", i + 1), m_Dependencies[i]);
    cfg.WriteString(
      group,
      wxString::Format("DependencyTitle%03d", i + 1),
      m_DependencyTitles[i]);
  }
}

void GOArchiveFile::Update(const GOArchiveFile &archive) {
  m_ID = archive.m_ID;
  if (m_Name != archive.m_Name)
    wxLogError(_("Organ package %s changed its title"), m_ID.c_str());
  m_Name = archive.m_Name;
  m_FileID = archive.m_FileID;
  m_Dependencies = archive.m_Dependencies;
}

wxString GOArchiveFile::GetCurrentFileID() const {
  GOHash hash;
  wxFileName path_name(m_Path);

  uint64_t size = path_name.GetSize().GetValue();
  uint64_t time = path_name.GetModificationTime().GetTicks();
  hash.Update(time);
  hash.Update(size);

  return hash.getStringHash();
}

const wxString &GOArchiveFile::GetID() const { return m_ID; }

const wxString &GOArchiveFile::GetPath() const { return m_Path; }

const wxString &GOArchiveFile::GetName() const { return m_Name; }

const wxString &GOArchiveFile::GetFileID() const { return m_FileID; }

const std::vector<wxString> &GOArchiveFile::GetDependencies() const {
  return m_Dependencies;
}

const std::vector<wxString> &GOArchiveFile::GetDependencyTitles() const {
  return m_DependencyTitles;
}

bool GOArchiveFile::IsUsable() const {
  return !m_Path.IsEmpty() && wxFileExists(m_Path);
}

bool GOArchiveFile::IsComplete(const GOOrganList &organs) const {
  bool isComplete = IsUsable();

  if (isComplete)
    for (auto &dep : m_Dependencies) {
      const GOArchiveFile *archive = organs.GetArchiveByID(dep, true);

      isComplete = archive && archive->IsUsable();

      if (!isComplete)
        break;
    }
  return isComplete;
}
