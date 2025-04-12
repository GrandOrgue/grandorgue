/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventPattern.h"

#include <algorithm>
#include <math.h>

bool GOMidiEventPattern::operator==(const GOMidiEventPattern &other) const {
  return deviceId == other.deviceId && channel == other.channel
    && key == other.key && low_value == other.low_value
    && high_value == other.high_value && useNoteOff == other.useNoteOff;
}

int GOMidiEventPattern::convertValueBetweenRanges(
  int srcValue, int srcLow, int srcHigh, int dstLow, int dstHigh) {
  const int dstAbsLow = std::min(dstLow, dstHigh);
  const int dstAbsHigh = std::max(dstLow, dstHigh);
  int dstValue = srcValue - srcLow;

  if (srcHigh != srcLow)
    dstValue = dstLow
      + round(dstValue * (float)(dstHigh - dstLow) / (srcHigh - srcLow));
  if (dstValue < dstAbsLow)
    dstValue = dstAbsLow;
  if (dstValue > dstAbsHigh)
    dstValue = dstAbsHigh;
  return dstValue;
}
