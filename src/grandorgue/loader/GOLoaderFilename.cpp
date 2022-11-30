/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLoaderFilename.h"

#include <wx/filename.h>
#include <wx/log.h>

#include "archive/GOArchive.h"
#include "config/GOConfig.h"
#include "loader/GOFileStore.h"

#include "GOHash.h"
#include "GOInvalidFile.h"
#include "GOStandardFile.h"

GOLoaderFilename::GOLoaderFilename()
  : m_Name(),
    m_Path(),
    m_Archiv(NULL),
    m_ToHashSizeTime(true),
    m_ToHashPath(true) {}

const wxString &GOLoaderFilename::GetTitle() const { return m_Name; }

void GOLoaderFilename::Hash(GOHash &hash) const {
  if (m_Archiv) {
    hash.Update(m_Archiv->GetArchiveID());
    hash.Update(m_Name);
    return;
  }
  hash.Update(m_ToHashPath ? m_Path : m_Name);
  wxFileName path_name(m_Path);
  if (!m_ToHashSizeTime)
    return;
  uint64_t size = path_name.GetSize().GetValue();
  uint64_t time = path_name.GetModificationTime().GetTicks();
  hash.Update(time);
  hash.Update(size);
}

std::unique_ptr<GOFile> GOLoaderFilename::Open() const {
  if (m_Archiv)
    return (std::unique_ptr<GOFile>)m_Archiv->OpenFile(m_Name);

  if (m_Path != wxEmptyString && wxFileExists(m_Path))
    return (std::unique_ptr<GOFile>)new GOStandardFile(m_Path, m_Name);
  else
    return (std::unique_ptr<GOFile>)new GOInvalidFile(m_Name);
}

void GOLoaderFilename::SetPath(const wxString &base, const wxString &file) {
  wxString temp = file;
  temp.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));
  temp = base + wxFileName::GetPathSeparator() + temp;

  wxFileName path_name(temp);
  path_name.Normalize(wxPATH_NORM_DOTS);
  if (path_name.GetLongPath() != path_name.GetFullPath()) {
    wxLogWarning(
      _("Filename '%s' not compatible with case sensitive systems"),
      file.c_str());
  }
  if (!wxFileExists(temp)) {
    wxLogError(_("File '%s' does not exists"), file.c_str());
    m_ToHashSizeTime = false;
  }
  m_Path = temp;
}

void GOLoaderFilename::Assign(
  const GOFileStore &fileStore, const wxString &name) {
  m_Name = name;
  if (fileStore.AreArchivesUsed()) {
    m_Path = wxEmptyString;
    m_Archiv = fileStore.FindArchiveContaining(name);
    if (!m_Archiv) {
      wxLogError(_("File '%s' does not exists"), name.c_str());
    }
    return;
  }
  SetPath(fileStore.GetDirectory(), name);
}

void GOLoaderFilename::AssignResource(
  const wxString &resourceDirectory, const wxString &name) {
  m_Name = name;
  m_ToHashSizeTime = false;
  m_ToHashPath = false;
  SetPath(resourceDirectory, name);
}

void GOLoaderFilename::AssignAbsolute(const wxString &path) {
  m_Name = path;
  m_Path = path;
  m_ToHashSizeTime = false;
}
