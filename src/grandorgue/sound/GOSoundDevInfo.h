/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDDEVINFO_H
#define GOSOUNDDEVINFO_H

#include <wx/string.h>

typedef struct {
  wxString name;
  unsigned channels;
  bool isDefault;
} GOSoundDevInfo;

#endif
