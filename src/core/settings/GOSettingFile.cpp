/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSettingFile.h"

#include <wx/filename.h>

GOSettingFile::GOSettingFile(GOSettingStore* store, wxString group, wxString name, wxString default_value) :
	GOSettingString(store, group, name, default_value)
{
}

wxString GOSettingFile::validate(wxString value)
{
	if (value == wxEmptyString || !wxFileExists(value))
		value = getDefaultValue();
	wxFileName file(value);
	file.MakeAbsolute();
	value = file.GetFullPath();
	return value;
}



