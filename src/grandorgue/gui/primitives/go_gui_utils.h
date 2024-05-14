/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GO_GUI_UTILS_H
#define GO_GUI_UTILS_H

#include <wx/colour.h>

#include "GOLogicalColour.h"

class wxIcon;

inline wxColour logicalToWxColour(const GOLogicalColour &lc) {
  return wxColour(lc.m_red, lc.m_green, lc.m_blue);
}

extern const wxIcon &get_go_icon();

#endif /* GO_GUI_UTILS_H */
