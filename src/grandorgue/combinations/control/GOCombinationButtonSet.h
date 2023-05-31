/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATIONBUTTONSET_H
#define GOCOMBINATIONBUTTONSET_H

class GOButtonControl;

class GOCombinationButtonSet {
protected:
  /**
   * Ubdate light of one button.
   * @param pButton - the button to control
   * @param buttonToLight - the button to light
   */
  void UpdateOneButtonLight(
    GOButtonControl *pButton, GOButtonControl *buttonToLight);

public:
  /**
   * Ubdate all combination buttons light.
   * If the button
   * @param buttonToLight
   * @param manualIndexOnlyFor
   */
  virtual void UpdateAllButtonsLight(
    GOButtonControl *buttonToLight, int manualIndexOnlyFor)
    = 0;
};

#endif /* GOCOMBINATIONBUTTONSET_H */
