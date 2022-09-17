/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBUTTONCALLBACK_H
#define GOBUTTONCALLBACK_H

class GOButtonControl;

class GOButtonCallback {
public:
  virtual ~GOButtonCallback() {}

  virtual void ButtonStateChanged(GOButtonControl *button) = 0;
};

#endif
