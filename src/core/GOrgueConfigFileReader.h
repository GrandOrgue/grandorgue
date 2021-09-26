/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECONFIGFILEREADER_H
#define GORGUECONFIGFILEREADER_H

#include <wx/string.h>
#include <map>

class GOrgueFile;

class GOrgueConfigFileReader
{
private:
	std::map<wxString, std::map<wxString, wxString> > m_Entries;
	wxString m_Hash;

	wxString GetNextLine(const wxString& buffer, unsigned &pos);
public:
	GOrgueConfigFileReader();
	~GOrgueConfigFileReader();
	
	bool Read(GOrgueFile* file);
	bool Read(wxString filename);
	wxString GetHash();

	const std::map<wxString, std::map<wxString, wxString> >& GetContent();
	wxString getEntry(wxString group, wxString name);
};

#endif
