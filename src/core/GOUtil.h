/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GO_UTIL_H
#define GO_UTIL_H

#include <wx/string.h>

wxString formatCDDouble(double value);
bool parseCDouble(double &result, wxString value);
bool parseLong(long &result, wxString value);

#endif
