/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIADDITIONALSIZEKEEPER_H
#define GOGUIADDITIONALSIZEKEEPER_H

#include <wx/string.h>

class GOGUIAdditionalSizeKeeper {
public:
  virtual int GetAddSize(const wxString &key) const = 0;
  virtual void SetAddSize(const wxString &key, int value) = 0;
};

#endif /* GOGUIADDITIONALSIZEKEEPER_H */
