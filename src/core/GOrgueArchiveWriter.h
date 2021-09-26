/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEARCHIVEWRITER_H
#define GORGUEARCHIVEWRITER_H

#include "GOrgueBuffer.h"
#include "GOrgueZipFormat.h"
#include <wx/file.h>
#include <vector>

class GOrgueArchiveWriter
{
private:
	wxFile m_File;
	size_t m_Offset;
	GOrgueBuffer<uint8_t> m_directory;
	unsigned m_Entries;
	std::vector<wxString> m_Names;

	bool Write(const void* data, size_t size);

public:
	GOrgueArchiveWriter();
	~GOrgueArchiveWriter();

	bool Open(wxString filename);
	bool Add(wxString name, const GOrgueBuffer<uint8_t>& content);
	bool Close();
};

#endif
