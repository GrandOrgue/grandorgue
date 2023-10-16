/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOArchiveManager.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfigFileReader.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "files/GOOpenedFile.h"

#include "GOArchive.h"
#include "GOArchiveFile.h"
#include "GOOrgan.h"
#include "GOOrganList.h"
#include "GOPath.h"

GOArchiveManager::GOArchiveManager(
  GOOrganList &OrganList, const wxString &cacheDir)
  : m_OrganList(OrganList), m_CacheDir(cacheDir) {}

GOArchiveManager::~GOArchiveManager() {}

GOArchive *GOArchiveManager::OpenArchive(const wxString &path) {
  GOArchive *archive = new GOArchive(m_CacheDir);

  if (!archive->OpenArchive(path)) {
    delete archive;
    return NULL;
  }
  return archive;
}

bool GOArchiveManager::ReadIndex(GOArchive *archive, bool InstallOrgans) {
  GOOpenedFile *indexFile = archive->OpenFile(wxT("organindex.ini"));
  if (!indexFile)
    return false;
  GOConfigFileReader ini_file;
  if (!ini_file.Read(indexFile)) {
    delete indexFile;
    return false;
  }
  delete indexFile;
  try {
    GOConfigReaderDB ini;
    ini.ReadData(ini_file, CMBSetting, false);
    GOConfigReader cfg(ini);
    wxString id = archive->GetArchiveID();

    wxString package_name
      = cfg.ReadString(CMBSetting, wxT("General"), wxT("Title"));
    unsigned dep_count = cfg.ReadInteger(
      CMBSetting, wxT("General"), wxT("DependencyCount"), 0, 100, false, 0);
    unsigned organ_count
      = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("OrganCount"), 0, 100);

    std::vector<wxString> depend;
    std::vector<wxString> depend_titles;
    for (unsigned i = 0; i < dep_count; i++) {
      wxString group = wxString::Format(wxT("Dependency%03d"), i + 1);
      wxString dep_id = cfg.ReadString(CMBSetting, group, wxT("PackageID"));
      wxString title = cfg.ReadString(CMBSetting, group, wxT("Title"));
      if (dep_id == id)
        throw(wxString) _("Self referencing organ package");
      depend.push_back(dep_id);
      depend_titles.push_back(title);
    }
    archive->SetDependencies(depend);

    std::vector<GOOrgan> organs;
    for (unsigned i = 0; i < organ_count; i++) {
      wxString group = wxString::Format(wxT("Organ%03d"), i + 1);
      wxString odf = cfg.ReadString(CMBSetting, group, wxT("Filename"));
      wxString ChurchName
        = cfg.ReadString(CMBSetting, group, wxT("ChurchName"));
      wxString OrganBuilder
        = cfg.ReadString(CMBSetting, group, wxT("OrganBuilder"), false);
      wxString RecordingDetails
        = cfg.ReadString(CMBSetting, group, wxT("RecordingDetails"), false);
      if (InstallOrgans)
        organs.push_back(GOOrgan(
          odf,
          id,
          archive->GetPath(),
          ChurchName,
          OrganBuilder,
          RecordingDetails));
    }

    GOArchiveFile a(
      id, archive->GetPath(), package_name, depend, depend_titles);
    m_OrganList.AddArchive(a);

    for (unsigned i = 0; i < organs.size(); i++)
      m_OrganList.AddOrgan(organs[i]);
  } catch (wxString error) {
    wxLogError(wxT("%s\n"), error.c_str());
    return false;
  }
  return true;
}

GOArchive *GOArchiveManager::LoadArchive(
  const wxString &id, const wxString &archivePath) {
  GOArchive *pResArchive = nullptr;

  for (const GOArchiveFile *file : m_OrganList.GetArchiveList()) {
    const wxString &filePath = file->GetPath();

    if (
      file->GetID() == id
      && (archivePath.IsEmpty() || filePath == archivePath)) {
      GOArchive *archiveProbe = OpenArchive(filePath);

      if (archiveProbe) {
        bool isSameOrgan = archiveProbe->GetArchiveID() == id;

        // if (!isSameOrgan) install organs from the archive but don't use it
        if (ReadIndex(archiveProbe, !isSameOrgan) && isSameOrgan) {
          pResArchive = archiveProbe;
          break;
        }
        delete archiveProbe;
      }
    }
  }
  return pResArchive;
}

wxString GOArchiveManager::InstallPackage(
  const wxString &path, const wxString &last_id) {
  wxString p = GONormalizePath(path);
  GOArchive *archive = OpenArchive(p);
  if (!archive)
    return wxString::Format(
      _("Failed to open the organ package '%s'"), path.c_str());
  bool result = ReadIndex(archive, last_id != archive->GetArchiveID());
  delete archive;
  if (!result)
    return wxString::Format(
      _("Failed to parse the organ package index of '%s'"), path.c_str());
  return wxEmptyString;
}

wxString GOArchiveManager::InstallPackage(const wxString &path) {
  return InstallPackage(path, wxEmptyString);
}

bool GOArchiveManager::RegisterPackage(const wxString &path) {
  wxString p = GONormalizePath(path);
  const GOArchiveFile *archive = m_OrganList.GetArchiveByPath(p);
  if (archive != NULL) {
    if (archive->GetFileID() == archive->GetCurrentFileID())
      return true;
  }
  wxString id;
  if (archive)
    id = archive->GetID();
  wxString result = InstallPackage(p, id);
  if (result != wxEmptyString) {
    wxLogError(_("%s"), result.c_str());
    return false;
  }
  return true;
}

void GOArchiveManager::RegisterPackageDirectory(const wxString &path) {
  wxDir dir(path);
  if (!dir.IsOpened()) {
    wxLogError(_("Failed to read directory '%s'"), path.c_str());
    return;
  }
  wxString name;
  if (!dir.GetFirst(&name, wxT("*.orgue"), wxDIR_FILES | wxDIR_HIDDEN))
    return;
  do {
    RegisterPackage(path + wxFileName::GetPathSeparator() + name);
  } while (dir.GetNext(&name));
}
