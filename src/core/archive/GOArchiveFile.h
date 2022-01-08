/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOARCHIVEFILE_H
#define GOARCHIVEFILE_H

#include <wx/string.h>

#include <vector>

class GOConfigReader;
class GOConfigWriter;
class GOOrganList;

class GOArchiveFile {
private:
  wxString m_ID;
  wxString m_FileID;
  wxString m_Path;
  wxString m_Name;
  std::vector<wxString> m_Dependencies;
  std::vector<wxString> m_DependencyTitles;

public:
  GOArchiveFile(
    wxString id,
    wxString path,
    wxString name,
    const std::vector<wxString> &dependencies,
    const std::vector<wxString> &dependency_titles);
  GOArchiveFile(GOConfigReader &cfg, wxString group);
  virtual ~GOArchiveFile();

  void Update(const GOArchiveFile &archive);
  wxString GetCurrentFileID() const;

  void Save(GOConfigWriter &cfg, wxString group);

  const wxString &GetID() const;
  const wxString &GetPath() const;
  const wxString &GetName() const;
  const wxString &GetFileID() const;
  const wxString GetArchiveHash() const;
  const std::vector<wxString> &GetDependencies() const;
  const std::vector<wxString> &GetDependencyTitles() const;

  bool IsUsable(const GOOrganList &organs) const;
  bool IsComplete(const GOOrganList &organs) const;
};

#endif
