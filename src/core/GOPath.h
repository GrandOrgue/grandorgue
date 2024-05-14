/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPATH_H
#define GOPATH_H

#include <wx/string.h>

void GOCreateDirectory(const wxString &path);
wxString GONormalizePath(const wxString &path);
wxString GOGetPath(const wxString &path);

void GOSyncDirectory(const wxString &path);
bool GORenameFile(const wxString &from, const wxString &to);

#endif
