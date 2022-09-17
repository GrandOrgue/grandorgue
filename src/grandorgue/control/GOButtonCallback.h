/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTERBUTTONCALLBACK_H
#define GOSETTERBUTTONCALLBACK_H

class GOSetterButtonControl;

class GOSetterButtonCallback {
public:
  virtual ~GOSetterButtonCallback() {}

  virtual void SetterButtonChanged(GOSetterButtonControl *button) = 0;
};

#endif
