/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingDirectory.h"

#include <wx/filename.h>

#include "GOPath.h"

GOSettingDirectory::GOSettingDirectory(
  GOSettingStore *store,
  const wxString &group,
  const wxString &name,
  const wxString &defaultValue)
  : GOSettingString(store, group, name, defaultValue) {}

wxString GOSettingDirectory::Validate(const wxString &value) const {
  wxString newValue = value;

  if (newValue == wxEmptyString || !wxFileName::DirExists(newValue))
    newValue = GetDefaultValue();

  wxFileName file(newValue);

  file.MakeAbsolute();
  newValue = file.GetFullPath();
  GOCreateDirectory(newValue);
  return newValue;
}
