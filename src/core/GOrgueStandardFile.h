/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESTANDARDFILE_H
#define GORGUESTANDARDFILE_H

#include "GOrgueFile.h"
#include <wx/file.h>

class GOrgueStandardFile : public GOrgueFile
{
private:
	wxString m_Path;
	wxString m_Name;
	wxFile m_File;
	size_t m_Size;

public:
	GOrgueStandardFile(const wxString& path);
	GOrgueStandardFile(const wxString& path, const wxString& name);
	~GOrgueStandardFile();

	size_t GetSize();
	const wxString GetName();
	const wxString GetPath();

	bool Open();
	void Close();
	size_t Read(void * buffer, size_t len);
};

#endif
