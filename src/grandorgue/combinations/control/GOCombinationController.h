/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
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
   * @param pButtonToLight the button to light on. All other buttons are
   *   ligthed off
   */
  virtual void PushGeneral(
    GOGeneralCombination &cmb, GOButtonControl *pButtonToLight)
    = 0;

  /**
   * Activate the divisional cmb. Does not process divisional couplers
   * @param cmb the cmb to activate or to set
   * @param startManual the manual that is pushed manually.
   * @param cmbManual the manual where to push the divisional. It may differ
   *   from startManual if cmbManual has a divisional coupler with startManual
   * @param pButtonToLight the button to light on. All other buttons on the
   *   cmbManual are ligthed off
   */
  virtual void PushDivisional(
    GODivisionalCombination &cmb,
    unsigned startManual,
    unsigned cmbManual,
    GOButtonControl *pButtonToLight)
    = 0;
};

#endif /* GOCOMBINATIONCONTROLLER_H */
