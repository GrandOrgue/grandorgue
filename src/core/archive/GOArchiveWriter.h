/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOARCHIVEWRITER_H
#define GOARCHIVEWRITER_H

#include <wx/file.h>

#include <vector>

#include "GOBuffer.h"
#include "GOZipFormat.h"

class GOArchiveWriter {
private:
  wxFile m_File;
  size_t m_Offset;
  GOBuffer<uint8_t> m_directory;
  unsigned m_Entries;
  std::vector<wxString> m_Names;

  bool Write(const void *data, size_t size);

public:
  GOArchiveWriter();
  ~GOArchiveWriter();

  bool Open(wxString filename);
  bool Add(wxString name, const GOBuffer<uint8_t> &content);
  bool Close();
};

#endif
