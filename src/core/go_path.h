/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPATH_H
#define GOPATH_H

#include <wx/string.h>

bool go_create_directory(const wxString &path);
wxString go_normalize_path(const wxString &path);
wxString go_get_path(const wxString &path);

void go_sync_directory(const wxString &path);
bool go_rename_file(const wxString &from, const wxString &to);

#endif
