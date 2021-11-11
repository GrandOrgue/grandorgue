/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMAINWINDOWDATA_H
#define GOMAINWINDOWDATA_H

#include "GOSaveableObject.h"
#include <wx/gdicmn.h>

class GODefinitionFile;

class GOMainWindowData : private GOSaveableObject
{
protected:
	GODefinitionFile* m_organfile;
	wxRect m_size;

	void Save(GOConfigWriter& cfg);

public:
	GOMainWindowData(GODefinitionFile* m_organfile);
	virtual ~GOMainWindowData();
	void Load(GOConfigReader& cfg, wxString group);

	wxRect GetWindowSize();
	void SetWindowSize(wxRect rect);
};

#endif
