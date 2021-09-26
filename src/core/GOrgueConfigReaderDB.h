/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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

	bool m_CaseSensitive;
	GOStringHashMap m_ODF;
	GOStringHashMap m_ODF_LC;
	GOStringHashMap m_CMB;
	GOBoolHashMap m_ODFUsed;
	GOBoolHashMap m_CMBUsed;

	void AddEntry(GOStringHashMap& hash, wxString key, wxString value);

public:
	GOrgueConfigReaderDB(bool case_sensitive = true);
	~GOrgueConfigReaderDB();
	void ReportUnused();
	void ClearCMB();
	bool ReadData(GOrgueConfigFileReader& ODF, GOSettingType type, bool handle_prefix);

	bool GetString(GOSettingType type, wxString group, wxString key, wxString& value);
};

#endif
