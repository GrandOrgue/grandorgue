/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCombinationButtonSet.h"

#include "control/GOButtonControl.h"

void GOCombinationButtonSet::updateOneButtonLight(
  GOButtonControl *pButton, GOButtonControl *buttonToLight) {
  if (pButton) {
    // switch off and then on is necessary for sending MIDI events
    pButton->Display(false);
    if (pButton == buttonToLight)
      pButton->Display(true);
  }
}
