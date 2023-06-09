/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATIONCONTROLLER_H
#define GOCOMBINATIONCONTROLLER_H

class GODivisionalCombination;
class GOGeneralCombination;
class GOButtonControl;

class GOCombinationController {
public:
  /*
   * Activate cmb
   * If the crescendo in add mode then not to disable stops that are present in
   * extraSet
   * If isFromCrescendo and it is in add mode then then does not depress other
   * buttons
   * @param cmb the cmb to activate or to set
   * @param pButton the button to light on
   * return if anything is changed
   */
  virtual void PushGeneral(
    GOGeneralCombination &cmb, GOButtonControl *pButtonToLight) = 0;
  virtual void PushDivisional(
    GODivisionalCombination &cmb, GOButtonControl *pButtonToLight) = 0;

};

#endif /* GOCOMBINATIONCONTROLLER_H */

