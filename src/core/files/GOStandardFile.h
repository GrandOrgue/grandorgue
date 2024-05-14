/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTANDARDFILE_H
#define GOSTANDARDFILE_H

#include <wx/file.h>

#include "GOOpenedFile.h"

class GOStandardFile : public GOOpenedFile {
private:
  wxString m_Path;
  wxString m_Name;
  wxFile m_File;
  size_t m_Size;

public:
  GOStandardFile(const wxString &path);
  GOStandardFile(const wxString &path, const wxString &name);
  ~GOStandardFile();

  size_t GetSize();
  const wxString GetName();
  const wxString GetPath();

  bool Open();
  void Close();
  size_t Read(void *buffer, size_t len);
};

#endif
