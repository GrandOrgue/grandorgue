/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMAINWINDOWDATA_H
#define GORGUEMAINWINDOWDATA_H

#include "GOrgueSaveableObject.h"
#include <wx/gdicmn.h>

class GrandOrgueFile;

class GOrgueMainWindowData : private GOrgueSaveableObject
{
protected:
	GrandOrgueFile* m_organfile;
	wxRect m_size;

	void Save(GOrgueConfigWriter& cfg);

public:
	GOrgueMainWindowData(GrandOrgueFile* m_organfile);
	virtual ~GOrgueMainWindowData();
	void Load(GOrgueConfigReader& cfg, wxString group);

	wxRect GetWindowSize();
	void SetWindowSize(wxRect rect);
};

#endif
