/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBUTTONCALLBACK_H
#define GOBUTTONCALLBACK_H

class GOButtonControl;

class GOButtonCallback {
public:
  virtual ~GOButtonCallback() {}

  /**
   * Called when the button is pushed of it's state is changed
   * @param button the button that is changed
   * @param newState is the button goes to be pushed or not.
   *   For pushbuttons it is always true
   */
  virtual void ButtonStateChanged(GOButtonControl *button, bool newState) = 0;
};

#endif
