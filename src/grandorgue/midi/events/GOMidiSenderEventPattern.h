/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDEREVENTPATTERN_H
#define GOMIDISENDEREVENTPATTERN_H

#include "GOMidiEventPattern.h"
#include "GOMidiSenderMessageType.h"

struct GOMidiSenderEventPattern : public GOMidiEventPattern {
  GOMidiSenderMessageType type;
  unsigned start;
  unsigned length;

  GOMidiSenderEventPattern()
    : GOMidiEventPattern(0, 1, 1, MIN_VALUE, MAX_VALUE),
      type(MIDI_S_NONE),
      start(0),
      length(0) {}

  bool operator==(const GOMidiSenderEventPattern &other) const;

  /**
   * Convert an internal midi value (from MIN_VALUE to MAX_VALUE) to a
   * destination one (from low_value to high_value)
   * @param intValue - internal midi value (from MIN_VALUE to MAX_VALUE)
   * @return Destination midi value (from low_value to high_value)
   */
  int ConvertIntValueToDst(int intValue) const {
    return convertValueBetweenRanges(
      intValue, MIN_VALUE, MAX_VALUE, low_value, high_value);
  }
};

#endif /* GOMIDISENDEREVENTPATTERN_H */
