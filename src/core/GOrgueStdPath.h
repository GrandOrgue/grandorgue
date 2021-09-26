/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESTDPATH_H
#define GORGUESTDPATH_H

#include <wx/string.h>

class GOrgueStdPath
{
public:
	static void InitLocaleDir();
	static wxString GetBaseDir();
	static wxString GetResourceDir();
	static wxString GetConfigDir();
	static wxString GetDocumentDir();
	static wxString GetCacheDir();
};

#endif
