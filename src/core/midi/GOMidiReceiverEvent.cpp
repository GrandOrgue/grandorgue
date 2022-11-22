/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiReceiverEvent.h"

#include <math.h>

int GOMidiReceiverEvent::GetNormalisedValue(int srcValue) {
  int value = srcValue - low_value;

  if (high_value != low_value)
    value = round(value * (float)127 / (high_value - low_value));
  if (value < 0)
    value = 0;
  if (value > 127)
    value = 127;
  return value;
}
