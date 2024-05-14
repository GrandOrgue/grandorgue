/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOARCHIVE_H
#define GOARCHIVE_H

#include <wx/file.h>
#include <wx/string.h>

#include <vector>

#include "threading/GOMutex.h"

class GOOpenedFile;
typedef struct _GOArchiveEntry GOArchiveEntry;

class GOArchive {
private:
  GOMutex m_Mutex;
  wxString m_CachePath;
  wxString m_ID;
  std::vector<wxString> m_Dependencies;
  std::vector<GOArchiveEntry> m_Entries;
  wxFile m_File;
  wxString m_Path;

public:
  GOArchive(const wxString &cachePath);
  ~GOArchive();

  bool OpenArchive(const wxString &path);
  void Close();

  bool containsFile(const wxString &name);
  GOOpenedFile *OpenFile(const wxString &name);

  size_t ReadContent(void *buffer, size_t offset, size_t len);

  const wxString &GetArchiveID();
  const wxString &GetPath();

  const std::vector<wxString> &GetDependencies() const;
  void SetDependencies(const std::vector<wxString> &dependencies);
};

#endif
