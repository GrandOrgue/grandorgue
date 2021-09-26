/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEARCHIVEINDEX_H
#define GORGUEARCHIVEINDEX_H

#include <wx/file.h>
#include <wx/string.h>
#include <vector>

class GOrgueSettingDirectory;
typedef struct _GOrgueHashType GOrgueHashType;

typedef struct _GOArchiveEntry
{
	wxString name;
	size_t offset;
	size_t len;
} GOArchiveEntry;

class GOrgueArchiveIndex
{
private:
	const GOrgueSettingDirectory& m_CachePath;
	wxString m_Path;
	wxFile m_File;

	GOrgueHashType GenerateHash();
	wxString GenerateIndexFilename();

	bool Write(const void* buf, unsigned len);
	bool Read(void* buf, unsigned len);

	bool WriteString(const wxString& str);
	bool ReadString(wxString& str);

	bool WriteEntry(const GOArchiveEntry& e);
	bool ReadEntry(GOArchiveEntry& e);

	bool ReadContent(wxString& id, std::vector<GOArchiveEntry>& entries);
	bool WriteContent(const wxString& id, const std::vector<GOArchiveEntry>& entries);

public:
	GOrgueArchiveIndex(const GOrgueSettingDirectory& cachePath, const wxString& path);
	~GOrgueArchiveIndex();

	bool ReadIndex(wxString& id, std::vector<GOArchiveEntry>& entries);
	bool WriteIndex(const wxString& id, const std::vector<GOArchiveEntry>& entries);
};

#endif
