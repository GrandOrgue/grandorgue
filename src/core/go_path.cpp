/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "go_path.h"

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/log.h>

bool go_create_directory(const wxString &path) {
  bool res = true;

  if (
    !wxFileName::DirExists(path)
    && !wxFileName::Mkdir(path, 0777, wxPATH_MKDIR_FULL)) {
    wxLogError(_("Failed to create directory '%s'"), path);
    res = false;
  }
  return res;
}

wxString go_normalize_path(const wxString &path) {
  wxFileName name(path);
  name.MakeAbsolute();
  return name.GetLongPath();
}

wxString go_get_path(const wxString &path) {
  wxFileName name(go_normalize_path(path));
  return name.GetPath();
}

void go_sync_directory(const wxString &path) {
  int fd = wxOpen(path.c_str(), O_RDONLY, 0);
  if (fd == -1)
    return;
  wxFsync(fd);
  wxClose(fd);
}

bool go_rename_file(const wxString &from, const wxString &to) {
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
  go_sync_directory(name.GetPath());
  return true;
}
