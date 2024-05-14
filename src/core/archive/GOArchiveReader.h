/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOARCHIVEREADER_H
#define GOARCHIVEREADER_H

#include <wx/string.h>

#include <vector>

#include "GOZipFormat.h"

class wxFile;
typedef struct _GOArchiveEntry GOArchiveEntry;

class GOArchiveReader {
private:
  wxFile &m_File;

  bool Seek(size_t offset);
  bool Read(void *buf, size_t len);
  bool GenerateFileHash(wxString &id);
  size_t ExtractU64(void *ptr);
  size_t ExtractU32(void *ptr);
  uint32_t CalculateCrc(size_t offset, size_t length);

  bool ReadFileRecord(
    size_t central_offset,
    GOZipCentralHeader &central,
    std::vector<GOArchiveEntry> &entries);
  bool ReadCentralDirectory(
    size_t offset,
    size_t entry_count,
    size_t length,
    std::vector<GOArchiveEntry> &entries);
  bool ReadEnd64Record(size_t offset, GOZipEnd64Record &record);
  bool ReadEndRecord(std::vector<GOArchiveEntry> &entries);

public:
  GOArchiveReader(wxFile &file);
  ~GOArchiveReader();

  bool ListContent(wxString &id, std::vector<GOArchiveEntry> &entries);
};

#endif
