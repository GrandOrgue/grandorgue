/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
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
  if (!m_File.Open(m_Path, wxFile::read)) return false;
  m_Size = m_File.Length();
  return true;
}
void GOStandardFile::Close() { m_File.Close(); }

size_t GOStandardFile::Read(void *buffer, size_t len) {
  ssize_t read = m_File.Read(buffer, len);
  if (read == wxInvalidOffset) return 0;
  return read;
}
