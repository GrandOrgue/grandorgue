/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOArchiveIndex.h"

#include <wx/filename.h>
#include <wx/log.h>

#include "files/GOStdFileName.h"

#include "GOArchiveFile.h"
#include "GOHash.h"

/* Value which is used to identify a valid cache index file. */
#define GRANDORGUE_INDEX_MAGIC 0x43214321

GOArchiveIndex::GOArchiveIndex(const wxString &cachePath, const wxString &path)
  : m_CachePath(cachePath), m_Path(path), m_File() {}

GOArchiveIndex::~GOArchiveIndex() { m_File.Close(); }

wxString GOArchiveIndex::GenerateIndexFilename() {
  return GOStdFileName::composeFullPath(
    m_CachePath,
    GOStdFileName::composeIndexFileName(GOArchiveFile::getArchiveHash(m_Path)));
}

GOHashType GOArchiveIndex::GenerateHash() {
  GOHash hash;
  hash.Update(sizeof(wxString));
  hash.Update(sizeof(GOArchiveEntry));
  hash.Update(sizeof(size_t));

  wxFileName path_name(m_Path);
  uint64_t size = path_name.GetSize().GetValue();
  uint64_t time = path_name.GetModificationTime().GetTicks();
  hash.Update(time);
  hash.Update(size);

  return hash.getHash();
}

bool GOArchiveIndex::Write(const void *buf, unsigned len) {
  return m_File.Write(buf, len) == len;
}

bool GOArchiveIndex::Read(void *buf, unsigned len) {
  return m_File.Read(buf, len) == len;
}

bool GOArchiveIndex::WriteString(const wxString &str) {
  size_t len = str.Length();
  if (!Write(&len, sizeof(len)))
    return false;
  if (!Write((const wxChar *)str.c_str(), len * sizeof(wxChar)))
    return false;
  return true;
}

bool GOArchiveIndex::ReadString(wxString &str) {
  size_t len;
  if (!Read(&len, sizeof(len)))
    return false;
  wxWCharBuffer buffer(len);
  if (!Read(buffer.data(), len * sizeof(wxChar)))
    return false;
  str = buffer;
  return true;
}

bool GOArchiveIndex::WriteEntry(const GOArchiveEntry &e) {
  if (!WriteString(e.name))
    return false;
  if (!Write(&e.offset, sizeof(e.offset)))
    return false;
  if (!Write(&e.len, sizeof(e.len)))
    return false;
  return true;
}

bool GOArchiveIndex::ReadEntry(GOArchiveEntry &e) {
  if (!ReadString(e.name))
    return false;
  if (!Read(&e.offset, sizeof(e.offset)))
    return false;
  if (!Read(&e.len, sizeof(e.len)))
    return false;
  return true;
}

bool GOArchiveIndex::ReadContent(
  wxString &id, std::vector<GOArchiveEntry> &entries) {
  if (!ReadString(id))
    return false;

  unsigned cnt;
  if (!Read(&cnt, sizeof(cnt)))
    return false;
  entries.resize(cnt);
  for (unsigned i = 0; i < entries.size(); i++)
    if (!ReadEntry(entries[i]))
      return false;

  return true;
}

bool GOArchiveIndex::WriteContent(
  const wxString &id, const std::vector<GOArchiveEntry> &entries) {
  int magic = GRANDORGUE_INDEX_MAGIC;
  if (!Write(&magic, sizeof(magic)))
    return false;

  GOHashType hash = GenerateHash();
  if (!Write(&hash, sizeof(hash)))
    return false;

  if (!WriteString(id))
    return false;

  unsigned cnt = entries.size();
  if (!Write(&cnt, sizeof(cnt)))
    return false;
  for (unsigned i = 0; i < entries.size(); i++)
    if (!WriteEntry(entries[i]))
      return false;

  return true;
}

bool GOArchiveIndex::ReadIndex(
  wxString &id, std::vector<GOArchiveEntry> &entries) {
  wxString name = GenerateIndexFilename();
  if (!wxFileExists(name))
    return false;
  if (!m_File.Open(name, wxFile::read)) {
    wxLogWarning(_("Failed to open '%s'"), name.c_str());
    return false;
  }

  int magic;
  GOHashType hash1, hash2;
  hash1 = GenerateHash();

  if (!Read(&magic, sizeof(magic)) || !Read(&hash2, sizeof(hash2))) {
    m_File.Close();
    wxLogWarning(_("Failed to read '%s'"), name.c_str());
    return false;
  }
  if (
    magic != GRANDORGUE_INDEX_MAGIC || memcmp(&hash1, &hash2, sizeof(hash1))) {
    m_File.Close();
    wxLogWarning(_("Index '%s' has bad magic - bypassing index"), name.c_str());
    return false;
  }

  if (!ReadContent(id, entries)) {
    m_File.Close();
    wxLogWarning(_("Failed to read '%s'"), name.c_str());
    return false;
  }

  m_File.Close();
  return true;
}

bool GOArchiveIndex::WriteIndex(
  const wxString &id, const std::vector<GOArchiveEntry> &entries) {
  wxString name = GenerateIndexFilename();
  if (!m_File.Create(name, true) || !m_File.IsOpened()) {
    m_File.Close();
    wxLogError(_("Failed to write to '%s'"), name.c_str());
    return false;
  }

  if (!WriteContent(id, entries)) {
    m_File.Close();
    wxLogError(_("Failed to write content to '%s'"), name.c_str());
    return false;
  }

  m_File.Close();
  return true;
}
