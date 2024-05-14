/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOArchive.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "files/GOInvalidFile.h"
#include "threading/GOMutexLocker.h"

#include "GOArchiveEntryFile.h"
#include "GOArchiveIndex.h"
#include "GOArchiveReader.h"

GOArchive::GOArchive(const wxString &cachePath)
  : m_CachePath(cachePath), m_ID(), m_Dependencies(), m_Entries(), m_Path() {}

GOArchive::~GOArchive() { Close(); }

bool GOArchive::OpenArchive(const wxString &path) {
  m_Entries.clear();
  m_Path = path;
  if (!m_File.Open(path, wxFile::read)) {
    wxLogError(_("Failed to open '%s'"), path.c_str());
    return false;
  }
  {
    GOArchiveIndex index(m_CachePath, m_Path);
    if (index.ReadIndex(m_ID, m_Entries))
      return true;
  }

  GOArchiveReader reader(m_File);
  if (!reader.ListContent(m_ID, m_Entries)) {
    wxLogError(_("Failed to parse '%s'"), path.c_str());
    return false;
  }

  GOArchiveIndex index(m_CachePath, m_Path);
  index.WriteIndex(m_ID, m_Entries);
  return true;
}

void GOArchive::Close() {
  m_File.Close();
  m_Entries.clear();
}

bool GOArchive::containsFile(const wxString &name) {
  for (unsigned i = 0; i < m_Entries.size(); i++)
    if (m_Entries[i].name == name)
      return true;
  return false;
}

GOOpenedFile *GOArchive::OpenFile(const wxString &name) {
  for (unsigned i = 0; i < m_Entries.size(); i++)
    if (m_Entries[i].name == name) {
      return new GOArchiveEntryFile(
        this, m_Entries[i].name, m_Entries[i].offset, m_Entries[i].len);
    }
  return new GOInvalidFile(name);
}

size_t GOArchive::ReadContent(void *buffer, size_t offset, size_t len) {
  GOMutexLocker lock(m_Mutex);
  ssize_t pos = m_File.Seek(offset);
  if (pos != (ssize_t)offset)
    return 0;
  ssize_t l = m_File.Read(buffer, len);
  if (l == wxInvalidOffset)
    return 0;
  return l;
}

const wxString &GOArchive::GetArchiveID() { return m_ID; }
const wxString &GOArchive::GetPath() { return m_Path; }

const std::vector<wxString> &GOArchive::GetDependencies() const {
  return m_Dependencies;
}

void GOArchive::SetDependencies(const std::vector<wxString> &dependencies) {
  m_Dependencies = dependencies;
}
