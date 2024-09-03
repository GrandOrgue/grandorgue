/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingDirectory.h"

#include <wx/filename.h>

#include "go_path.h"

GOSettingDirectory::GOSettingDirectory(
  GOSettingStore *store,
  const wxString &group,
  const wxString &name,
  const wxString &defaultValue)
  : GOSettingString(store, group, name, defaultValue) {}

wxString GOSettingDirectory::Validate(const wxString &value) const {
  wxFileName file(!value.IsEmpty() ? value : GetDefaultValue());

  file.MakeAbsolute();

  const wxString newValue = file.GetFullPath();

  if (!wxFileName::DirExists(newValue))
    go_create_directory(newValue);
  return newValue;
}
