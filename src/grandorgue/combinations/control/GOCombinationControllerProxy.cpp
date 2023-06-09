/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombinationControllerProxy.h"

void GOCombinationControllerProxy::PushGeneral(
  GOGeneralCombination &cmb, GOButtonControl *pButtonToLight) {
  if (p_controller)
    p_controller->PushGeneral(cmb, pButtonToLight);
}

void GOCombinationControllerProxy::PushDivisional(
  GODivisionalCombination &cmb, GOButtonControl *pButtonToLight) {
  if (p_controller)
    p_controller->PushDivisional(cmb, pButtonToLight);
}
