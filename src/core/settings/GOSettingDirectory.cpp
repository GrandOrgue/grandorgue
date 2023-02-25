/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingDirectory.h"

#include <wx/filename.h>

#include "GOPath.h"

GOSettingDirectory::GOSettingDirectory(
  GOSettingStore *store, wxString group, wxString name, wxString default_value)
  : GOSettingString(store, group, name, default_value) {}

wxString GOSettingDirectory::validate(wxString value) {
  if (value == wxEmptyString || !wxFileName::DirExists(value))
    value = getDefaultValue();
  wxFileName file(value);
  file.MakeAbsolute();
  value = file.GetFullPath();
  GOCreateDirectory(value);
  return value;
}
