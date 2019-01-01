/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
