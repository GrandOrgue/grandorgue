/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingFile.h"

#include <wx/filename.h>

GOSettingFile::GOSettingFile(
  GOSettingStore *store,
  const wxString &group,
  const wxString &name,
  const wxString &defaultValue)
  : GOSettingString(store, group, name, defaultValue) {}

wxString GOSettingFile::Validate(const wxString &value) const {
  wxString newValue = value;

  if (newValue == wxEmptyString || !wxFileExists(newValue))
    newValue = GetDefaultValue();
  if (!newValue.IsEmpty()) {
    // convert value to an absolute path
    wxFileName file(newValue);

    file.MakeAbsolute();
    newValue = file.GetFullPath();
  }
  return newValue;
}
