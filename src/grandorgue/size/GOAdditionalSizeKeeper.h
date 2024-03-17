/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOADDITIONALSIZEKEEPER_H
#define GOADDITIONALSIZEKEEPER_H

#include <wx/string.h>

/**
 * An abstract class for storing information of sizes
 */
class GOAdditionalSizeKeeper {
public:
  virtual int GetAdditionalSize(const wxString &key) const = 0;
  virtual void SetAdditionalSize(const wxString &key, int value) = 0;
};

#endif /* GOADDITIONALSIZEKEEPER_H */
