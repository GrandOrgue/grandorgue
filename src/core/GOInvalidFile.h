/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOINVALIDFILE_H
#define GOINVALIDFILE_H

#include "GOFile.h"

class GOInvalidFile : public GOFile {
private:
  wxString m_Name;

public:
  GOInvalidFile(const wxString &name);

  bool isValid();
  size_t GetSize();
  const wxString GetName();
  const wxString GetPath();

  bool Open();
  void Close();
  size_t Read(void *buffer, size_t len);
};

#endif
