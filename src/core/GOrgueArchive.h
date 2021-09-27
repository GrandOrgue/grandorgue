/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEARCHIVE_H
#define GORGUEARCHIVE_H

#include "mutex.h"
#include <wx/file.h>
#include <wx/string.h>
#include <vector>

class GOrgueFile;
class GOrgueSettingDirectory;
typedef struct _GOArchiveEntry GOArchiveEntry;

class GOrgueArchive
{
private:
	GOMutex m_Mutex;
	const GOrgueSettingDirectory& m_CachePath;
	wxString m_ID;
	std::vector<wxString> m_Dependencies;
	std::vector<GOArchiveEntry> m_Entries;
	wxFile m_File;
	wxString m_Path;

public:
	GOrgueArchive(const GOrgueSettingDirectory& cachePath);
	~GOrgueArchive();

	bool OpenArchive(const wxString& path);
	void Close();

	bool containsFile(const wxString& name);
	GOrgueFile* OpenFile(const wxString& name);

	size_t ReadContent(void* buffer, size_t offset, size_t len);

	const wxString& GetArchiveID();
	const wxString& GetPath();

	const std::vector<wxString>& GetDependencies() const;
	void SetDependencies(const std::vector<wxString>& dependencies);
};

#endif
