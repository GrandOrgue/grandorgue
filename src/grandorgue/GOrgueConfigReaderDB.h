/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUECONFIGREADERDB_H
#define GORGUECONFIGREADERDB_H

#include "GOrgueConfigReader.h"
#include <wx/hashmap.h>
#include <wx/string.h>

class GOrgueConfigFileReader;

class GOrgueConfigReaderDB
{
private:
	WX_DECLARE_STRING_HASH_MAP( wxString, GOStringHashMap );
	WX_DECLARE_STRING_HASH_MAP( bool, GOBoolHashMap );

	GOStringHashMap m_ODF;
	GOStringHashMap m_ODF_LC;
	GOStringHashMap m_CMB;
	GOBoolHashMap m_ODFUsed;
	GOBoolHashMap m_CMBUsed;

	void AddEntry(GOStringHashMap& hash, wxString key, wxString value);

public:
	GOrgueConfigReaderDB();
	~GOrgueConfigReaderDB();
	void ReportUnused();
	void ClearCMB();
	bool ReadData(GOrgueConfigFileReader& ODF, GOSettingType type, bool handle_prefix);

	bool GetString(GOSettingType type, wxString group, wxString key, wxString& value);
};

#endif
