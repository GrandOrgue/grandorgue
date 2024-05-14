/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOFileStore.h"

#include <wx/intl.h>

#include "archive/GOArchive.h"
#include "archive/GOArchiveFile.h"
#include "archive/GOArchiveManager.h"
#include "config/GOConfig.h"

#include "GOOrganList.h"

GOFileStore::GOFileStore(const GOConfig &config)
  : m_ResourceDirectory(config.GetResourceDirectory()) {}

void GOFileStore::SetDirectory(const wxString &directory) {
  // we do not support if both m_directory and m_archives are filled
  m_archives.clear();
  m_directory = directory;
}

bool GOFileStore::LoadArchive(
  GOOrganList &organList,
  const wxString &cacheDir,
  const wxString &id,
  const wxString &archivePath,
  const wxString &parentId,
  wxString &errMsg) {
  bool isOk = true;
  GOArchiveManager manager(organList, cacheDir);
  GOArchive *archive = manager.LoadArchive(id, archivePath);

  if (archive)
    m_archives.push_back(archive);
  else {
    // get the archive name for composing an error message
    wxString archiveName;
    const GOArchiveFile *a = organList.GetArchiveByID(id);

    if (a)
      archiveName = a->GetName();
    else if (parentId != wxEmptyString) {
      a = organList.GetArchiveByID(parentId);
      for (unsigned i = 0; i < a->GetDependencies().size(); i++)
        if (a->GetDependencies()[i] == id)
          archiveName = a->GetDependencyTitles()[i];
    }

    errMsg = wxString::Format(
      _("Failed to open organ package '%s' (%s)"), archiveName, id);
    isOk = false;
  }
  return isOk;
}

bool GOFileStore::LoadArchives(
  GOOrganList &organList,
  const wxString &cacheDir,
  const wxString &mainArchiveId,
  const wxString &mainArchivePath,
  wxString &errMsg) {
  SetDirectory(wxEmptyString); // clean up all

  bool isOk = LoadArchive(
    organList, cacheDir, mainArchiveId, mainArchivePath, wxEmptyString, errMsg);

  if (isOk)
    for (auto depId : m_archives[0]->GetDependencies()) {
      isOk = LoadArchive(
        organList, cacheDir, depId, wxEmptyString, mainArchiveId, errMsg);
      if (!isOk)
        break;
    }
  return isOk;
}

GOArchive *GOFileStore::FindArchiveContaining(const wxString fileName) const {
  GOArchive *aFound = nullptr;

  for (auto a : m_archives) {
    if (a->containsFile(fileName)) {
      aFound = a;
      break;
    }
  }
  return aFound;
}

void GOFileStore::CloseArchives() {
  for (auto a : m_archives)
    a->Close();
}
