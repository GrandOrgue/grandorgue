/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATIONCONTROLLERPROXY_H
#define GOCOMBINATIONCONTROLLERPROXY_H

#include "GOCombinationController.h"

class GOCombinationControllerProxy : public GOCombinationController {
private:
  GOCombinationController *p_controller = nullptr;

public:
  void SetCombinationController(GOCombinationController *pController) {
    p_controller = pController;
  }

  void PushGeneral(
    GOGeneralCombination &cmb, GOButtonControl *pButtonToLight) override;
  void PushDivisional(
    GODivisionalCombination &cmb,
    unsigned startManual,
    unsigned cmbManual,
    GOButtonControl *pButtonToLight) override;
};

#endif /* GOCOMBINATIONCONTROLLERPROXY_H */
