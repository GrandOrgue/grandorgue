/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATIONELEMENT_H
#define GOCOMBINATIONELEMENT_H

class GOCombinationElement {
public:
  virtual bool GetCombinationState() const = 0;
  virtual void SetCombinationState(bool on) = 0;
  virtual bool IsControlledByUser() const = 0;
};

#endif
