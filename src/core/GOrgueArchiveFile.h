/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEARCHIVEFILE_H
#define GORGUEARCHIVEFILE_H

#include <wx/string.h>
#include <vector>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueOrganList;

class GOrgueArchiveFile
{
private:
	wxString m_ID;
	wxString m_FileID;
	wxString m_Path;
	wxString m_Name;
	std::vector<wxString> m_Dependencies;
	std::vector<wxString> m_DependencyTitles;

public:
	GOrgueArchiveFile(wxString id, wxString path, wxString name, const std::vector<wxString>& dependencies, const std::vector<wxString>& dependency_titles);
	GOrgueArchiveFile(GOrgueConfigReader& cfg, wxString group);
	virtual ~GOrgueArchiveFile();

	void Update(const GOrgueArchiveFile& archive);
	wxString GetCurrentFileID() const;

	void Save(GOrgueConfigWriter& cfg, wxString group);

	const wxString& GetID() const;
	const wxString& GetPath() const;
	const wxString& GetName() const;
	const wxString& GetFileID() const;
	const wxString GetArchiveHash() const;
	const std::vector<wxString>& GetDependencies() const;
	const std::vector<wxString>& GetDependencyTitles() const;

	bool IsUsable(const GOrgueOrganList& organs) const;
	bool IsComplete(const GOrgueOrganList& organs) const;
};

#endif
