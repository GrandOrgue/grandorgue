/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOADERFILENAME_H
#define GOLOADERFILENAME_H

#include <wx/string.h>

#include <memory>

class GOArchive;
class GOFile;
class GOFileStore;
class GOHash;

class GOLoaderFilename {
private:
  wxString m_Name;
  wxString m_Path;
  GOArchive *m_Archiv;
  bool m_ToHashSizeTime;
  bool m_ToHashPath;

  void SetPath(const wxString &base, const wxString &path);

public:
  GOLoaderFilename();

  void Assign(const GOFileStore &fileStore, const wxString &name);
  void AssignResource(const wxString &resourceDirectory, const wxString &name);
  void AssignAbsolute(const wxString &path);

  const wxString &GetTitle() const;
  void Hash(GOHash &hash) const;

  std::unique_ptr<GOFile> Open() const;
};

#endif
