/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLoaderFilename.h"

#include <wx/filename.h>
#include <wx/log.h>

#include "archive/GOArchive.h"
#include "files/GOStandardFile.h"
#include "loader/GOFileStore.h"

#include "GOHash.h"

void GOLoaderFilename::Assign(const RootKind rootKind, const wxString &path) {
  wxFileName tmpPath(path);

  m_RootKind = rootKind;
  tmpPath.Normalize(wxPATH_NORM_DOTS);
  m_path = tmpPath.GetFullPath();
  if (m_path != tmpPath.GetLongPath())
    wxLogWarning(
      _("Filename '%s' not compatible with case sensitive systems"), m_path);
}

void GOLoaderFilename::Hash(GOHash &hash) const {
  hash.Update(m_RootKind);
  hash.Update(m_path);
}

std::unique_ptr<GOOpenedFile> GOLoaderFilename::Open(
  const GOFileStore &fileStore) const {
  GOOpenedFile *file;

  assert(m_RootKind != ROOT_UNKNOWN);
  if (m_RootKind == ROOT_ODF && fileStore.AreArchivesUsed()) {
    GOArchive *const archive = fileStore.FindArchiveContaining(m_path);

    if (!archive)
      throw wxString::Format(
        _("File %s is not found in the organ package archives"), m_path);
    file = archive->OpenFile(m_path);
  } else {
    wxString baseDir;

    if (m_RootKind == ROOT_ODF)
      baseDir = fileStore.GetDirectory();
    else if (m_RootKind == ROOT_RESOURCE)
      baseDir = fileStore.GetResourceDirectory();

    wxString fullPath = m_path;

    fullPath.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));
    if (!baseDir.IsEmpty())
      fullPath = baseDir + wxFileName::GetPathSeparator() + fullPath;

    if (fullPath.IsEmpty())
      throw _("File name is empty");
    if (!wxFileExists(fullPath))
      throw wxString::Format(_("File '%s' does not exist"), fullPath);
    file = new GOStandardFile(fullPath, m_path);
  }
  return std::unique_ptr<GOOpenedFile>(file);
}
