/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEKEYCONVERT_H
#define GORGUEKEYCONVERT_H

#include <wx/string.h>

typedef struct {
	wxString name;
	unsigned key_code;
} GOShortcutKey;

unsigned GetShortcutKeyCount();
const GOShortcutKey* GetShortcutKeys();

int WXKtoVK(int what);

#endif
