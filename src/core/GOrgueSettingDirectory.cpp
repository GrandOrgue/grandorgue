/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSettingDirectory.h"

#include "GOrguePath.h"
#include <wx/filename.h>

GOrgueSettingDirectory::GOrgueSettingDirectory(GOrgueSettingStore* store, wxString group, wxString name, wxString default_value) :
	GOrgueSettingString(store, group, name, default_value)
{
}

wxString GOrgueSettingDirectory::validate(wxString value)
{
	if (value == wxEmptyString || !wxFileName::DirExists(value))
		value = getDefaultValue();
	wxFileName file(value);
	file.MakeAbsolute();
	value = file.GetFullPath();
	GOCreateDirectory(value);
	return value;
}
