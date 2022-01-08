/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOFILENAME_H
#define GOFILENAME_H

#include <wx/string.h>

#include <memory>

class GOArchive;
class GOFile;
class GOHash;
class GODefinitionFile;

class GOFilename {
 private:
  wxString m_Name;
  wxString m_Path;
  GOArchive *m_Archiv;
  bool m_Hash;

  void SetPath(const wxString &base, const wxString &path);

 public:
  GOFilename();

  void Assign(const wxString &name, GODefinitionFile *organfile);
  void AssignResource(const wxString &name, GODefinitionFile *organfile);
  void AssignAbsolute(const wxString &path);

  const wxString &GetTitle() const;
  void Hash(GOHash &hash) const;

  std::unique_ptr<GOFile> Open() const;
};

#endif
