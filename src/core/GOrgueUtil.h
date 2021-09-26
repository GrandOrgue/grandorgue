/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUE_UTIL_H
#define GORGUE_UTIL_H

#include <wx/string.h>
#include <wx/colour.h>

wxString formatCDDouble(double value);
bool parseCDouble(double& result, wxString value);
bool parseLong(long& result, wxString value);
bool parseColor(wxColour& result, wxString value);

#endif
