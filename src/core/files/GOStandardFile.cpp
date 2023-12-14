/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOStandardFile.h"

GOStandardFile::GOStandardFile(const wxString &path)
  : m_Path(path), m_Name(path), m_Size(0) {}

GOStandardFile::GOStandardFile(const wxString &path, const wxString &name)
  : m_Path(path), m_Name(name), m_Size(0) {}

GOStandardFile::~GOStandardFile() { Close(); }

const wxString GOStandardFile::GetName() { return m_Name; }

const wxString GOStandardFile::GetPath() { return m_Path; }

size_t GOStandardFile::GetSize() { return m_Size; }

bool GOStandardFile::Open() {
  bool isOk = false;

  // Avoid opening a directory
  if (wxFileExists(m_Path) && m_File.Open(m_Path, wxFile::read)) {
    m_Size = m_File.Length();
    isOk = true;
  }
  return isOk;
}
void GOStandardFile::Close() { m_File.Close(); }

size_t GOStandardFile::Read(void *buffer, size_t len) {
  ssize_t read = m_File.Read(buffer, len);
  if (read == wxInvalidOffset)
    return 0;
  return read;
}
