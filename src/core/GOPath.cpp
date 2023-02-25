/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPath.h"

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/log.h>

void GOCreateDirectory(const wxString &path) {
  if (wxFileName::DirExists(path))
    return;
  if (!wxFileName::Mkdir(path, 0777, wxPATH_MKDIR_FULL)) {
    wxLogError(_("Failed to create directory '%s'"), path.c_str());
  }
}

wxString GONormalizePath(const wxString &path) {
  wxFileName name(path);
  name.MakeAbsolute();
  return name.GetLongPath();
}

wxString GOGetPath(const wxString &path) {
  wxFileName name(GONormalizePath(path));
  return name.GetPath();
}

void GOSyncDirectory(const wxString &path) {
  int fd = wxOpen(path.c_str(), O_RDONLY, 0);
  if (fd == -1)
    return;
  wxFsync(fd);
  wxClose(fd);
}

bool GORenameFile(const wxString &from, const wxString &to) {
  wxFileName name(to);
  if (!wxRenameFile(from, to)) {
    if (::wxFileExists(to) && !::wxRemoveFile(to)) {
      wxLogError(_("Could not remove '%s'"), to.c_str());
      return false;
    }
    if (!wxRenameFile(from, to)) {
      wxLogError(_("Could not write to '%s'"), to.c_str());
      return false;
    }
  }
  GOSyncDirectory(name.GetPath());
  return true;
}
