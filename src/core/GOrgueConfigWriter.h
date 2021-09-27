/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECONFIGWRITER_H
#define GORGUECONFIGWRITER_H

#include <wx/string.h>

class GOrgueConfigFileWriter;
struct IniFileEnumEntry;

class GOrgueConfigWriter
{
private:
	GOrgueConfigFileWriter& m_ConfigFile;
	bool m_Prefix;

public:
	GOrgueConfigWriter(GOrgueConfigFileWriter& cfg, bool prefix);

	void WriteString(wxString group, wxString key, wxString value);
	void WriteInteger(wxString group, wxString key, int value);
	void WriteEnum(wxString group, wxString key, int value, const struct IniFileEnumEntry* entry, unsigned count);
	void WriteFloat(wxString group, wxString key, float value);
	void WriteBoolean(wxString group, wxString key, bool value);
};

#endif
