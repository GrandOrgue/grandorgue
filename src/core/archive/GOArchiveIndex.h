/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOARCHIVEINDEX_H
#define GOARCHIVEINDEX_H

#include <wx/file.h>
#include <wx/string.h>

#include <vector>

class GOSettingDirectory;
typedef struct _GOHashType GOHashType;

typedef struct _GOArchiveEntry {
  wxString name;
  size_t offset;
  size_t len;
} GOArchiveEntry;

class GOArchiveIndex {
private:
  wxString m_CachePath;
  wxString m_Path;
  wxFile m_File;

  GOHashType GenerateHash();
  wxString GenerateIndexFilename();

  bool Write(const void *buf, unsigned len);
  bool Read(void *buf, unsigned len);

  bool WriteString(const wxString &str);
  bool ReadString(wxString &str);

  bool WriteEntry(const GOArchiveEntry &e);
  bool ReadEntry(GOArchiveEntry &e);

  bool ReadContent(wxString &id, std::vector<GOArchiveEntry> &entries);
  bool WriteContent(
    const wxString &id, const std::vector<GOArchiveEntry> &entries);

public:
  GOArchiveIndex(const wxString &cachePath, const wxString &path);
  ~GOArchiveIndex();

  bool ReadIndex(wxString &id, std::vector<GOArchiveEntry> &entries);
  bool WriteIndex(
    const wxString &id, const std::vector<GOArchiveEntry> &entries);
};

#endif
