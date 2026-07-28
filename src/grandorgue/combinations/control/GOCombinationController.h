/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATIONCONTROLLER_H
#define GOCOMBINATIONCONTROLLER_H

#include <functional>

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
   * Activate or set the divisional cmb for all manuals coupled with
   * startManualIndex
   * @param startManualIndex the manual that is pushed manually
   * @param findManualDivisional the function (usually lambda) returning a pair
   *   of (pButtonToLight, pCmb) by coupledManualIndex. Any of them may be
   *   nullptr. pButtonToLight is a pointer to the button to light on.
   *   All other buttons on the cmbManual are ligthed off. pCmb is a pointer to
   *   the combination to activate or to set.
   */
  virtual void ProcessPushDivisional(
    unsigned startManualIndex,
    std::function<std::pair<GOButtonControl *, GODivisionalCombination *>(
      unsigned)> findManualDivisional)
    = 0;
};

#endif /* GOCOMBINATIONCONTROLLER_H */
