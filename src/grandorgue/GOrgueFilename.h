/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEFILENAME_H
#define GORGUEFILENAME_H

#include <wx/string.h>
#include <memory>

class GOrgueArchive;
class GOrgueFile;
class GOrgueHash;
class GrandOrgueFile;

class GOrgueFilename
{
private:
	wxString m_Name;
	wxString m_Path;
	GOrgueArchive* m_Archiv;
	bool m_Hash;

	void SetPath(const wxString& base, const wxString& path);
	
public:
	GOrgueFilename();

	void Assign(const wxString& name, GrandOrgueFile* organfile);
	void AssignResource(const wxString& name, GrandOrgueFile* organfile);
	void AssignAbsolute(const wxString& path);

	const wxString& GetTitle() const;
	void Hash(GOrgueHash& hash) const;

	std::unique_ptr<GOrgueFile> Open() const;
};

#endif
