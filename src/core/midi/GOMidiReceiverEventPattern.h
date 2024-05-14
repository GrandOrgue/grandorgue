/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVEREVENTPATTERN_H
#define GOMIDIRECEIVEREVENTPATTERN_H

#include "GOMidiEventPattern.h"
#include "GOMidiReceiverMessageType.h"

struct GOMidiReceiverEventPattern : public GOMidiEventPattern {
  GOMidiReceiverMessageType type = MIDI_M_NONE;
  int low_key;
  int high_key;
  unsigned debounce_time;

  GOMidiReceiverEventPattern()
    : GOMidiEventPattern(0, -1, 0),
      type(MIDI_M_NONE),
      low_key(0),
      high_key(0),
      debounce_time(0) {}

  bool operator==(const GOMidiReceiverEventPattern &other) const;

  /**
   * Convert a source midi value (from low_value to high_value) to an internal
   *   one (from MIN_VALUE to MAX_VALUE)
   * @param srcValue - source midi value (from low_value to high_value)
   * @return normalised midi value (from MIN_VALUE to MAX_VALUE)
   */
  int ConvertSrcValueToInt(int srcValue) const {
    return convertValueBetweenRanges(
      srcValue, low_value, high_value, MIN_VALUE, MAX_VALUE);
  }
};

#endif /* GOMIDIRECEIVEREVENTPATTERN_H */
