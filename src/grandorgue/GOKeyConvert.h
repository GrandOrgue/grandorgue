/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOKEYCONVERT_H
#define GOKEYCONVERT_H

#include <wx/string.h>

class GOKeyConvert {
public:
  struct Shortcut {
    wxString name;
    unsigned key_code;
  };

  static unsigned getShortcutKeyCount();
  static const Shortcut *getShortcutKeys();

  static int wXKtoVK(int what);
};

#endif
