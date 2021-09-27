/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECONFIGFILEWRITER_H
#define GORGUECONFIGFILEWRITER_H

#include <wx/string.h>
#include <map>

template<class T>
class GOrgueBuffer;

class GOrgueConfigFileWriter
{
private:
	std::map<wxString, std::map<wxString, wxString> > m_Entries;

public:
	GOrgueConfigFileWriter();
	
	void AddEntry(wxString group, wxString name, wxString value);
	bool GetFileContent(GOrgueBuffer<uint8_t>& buffer);
	bool Save(wxString filename);
};

#endif
