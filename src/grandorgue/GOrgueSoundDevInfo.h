/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESOUNDDEVINFO_H
#define GORGUESOUNDDEVINFO_H

#include <wx/string.h>

typedef struct
{
	wxString name;
	unsigned channels;
	bool isDefault;
} GOrgueSoundDevInfo;

#endif
