/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GORESIZABLE_H
#define GORESIZABLE_H

#include "GOLogicalRect.h"

class GOResizable {
public:
  virtual GOLogicalRect GetPosSize() const = 0;
  virtual void SetPosSize(const GOLogicalRect &rect) = 0;
};

#endif /* GORESIZABLE_H */
