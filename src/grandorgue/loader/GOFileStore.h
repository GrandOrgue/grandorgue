/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOFILESTORE_H
#define GOFILESTORE_H

#include <wx/string.h>

#include "ptrvector.h"

class GOArchive;
class GOOrganList;

/**
 * This class represents a container that contains other files
 * It can be either a directory or a list of (dependent) archives
 */

class GOConfig;

class GOFileStore {
private:
  wxString m_ResourceDirectory;

  wxString m_directory;
  ptr_vector<GOArchive> m_archives;
  bool LoadArchive(
    GOOrganList &organList,
    const wxString &cacheDir,
    const wxString &id,
    const wxString &archivePath,
    const wxString &parentId,
    wxString &errMsg);

public:
  GOFileStore(const GOConfig &config);

  const wxString &GetResourceDirectory() const { return m_ResourceDirectory; }
  const wxString &GetDirectory() const { return m_directory; }
  void SetDirectory(const wxString &directory);
  bool AreArchivesUsed() const { return m_archives.size() > 0; }
  const GOArchive *GetMainArchive() const {
    return AreArchivesUsed() ? m_archives[0] : nullptr;
  }

  /**
   * Load archive directories for the given archive id and all it's dependencies
   * @param organList - a list of registered organs
   * @param cacheDir - a directory to read/write index files
   * @param mainArchiveId - an identifier of the main archive to load
   * @param mainArchivePath - an path to the main archive to load. If empty
   *   then search among organList
   * @param errMsg - an error message that is returned if the operation fails
   * @return - true if success and false if any error occured (the error message
   *   is returned in errMsg)
   */
  bool LoadArchives(
    GOOrganList &organList,
    const wxString &cacheDir,
    const wxString &mainArchiveId,
    const wxString &mainArchivePath,
    wxString &errMsg);

  /**
   * search fileName across all archives in m_archives
   * @param fileName - a file name to search
   * @return a pointer to an archive containing the file or nullptr if the file
   *   does not exist in any archive
   */
  GOArchive *FindArchiveContaining(const wxString fileName) const;

  void CloseArchives();
};

#endif /* GOFILESTORE_H */
