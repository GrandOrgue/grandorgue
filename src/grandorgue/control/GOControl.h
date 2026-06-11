/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONTROL_H
#define GOCONTROL_H

/* The base class of other controls */

class GOControl {
public:
  virtual ~GOControl() {}

  virtual bool IsKeyboardInputUsed() const { return false; }
};

#endif /* GOCONTROL_H */
