/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEBITMAPCACHE_H
#define GORGUEBITMAPCACHE_H

#include "ptrvector.h"
#include "GOrgueBitmap.h"
#include <wx/string.h>

class GrandOrgueFile;

class GOrgueBitmapCache
{
private:
	GrandOrgueFile* m_organfile;
	ptr_vector<wxImage> m_Bitmaps;
	std::vector<wxString> m_Filenames;
	std::vector<wxString> m_Masknames;

	bool loadFile(wxImage& img, wxString filename);

public:
	GOrgueBitmapCache(GrandOrgueFile* organfile);
	virtual ~GOrgueBitmapCache();

	void RegisterBitmap(wxImage* bitmap, wxString filename, wxString maskname = wxEmptyString);
	GOrgueBitmap GetBitmap(wxString filename, wxString maskName = wxEmptyString);
};

#endif
