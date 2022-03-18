/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTERBUTTONCALLBACK_H
#define GOSETTERBUTTONCALLBACK_H

class GOSetterButton;

class GOSetterButtonCallback {
public:
  virtual ~GOSetterButtonCallback() {}

  virtual void SetterButtonChanged(GOSetterButton *button) = 0;
};

#endif
