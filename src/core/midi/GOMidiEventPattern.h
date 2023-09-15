/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTPATTERN_H
#define GOMIDIEVENTPATTERN_H

struct GOMidiEventPattern {
  enum { MIN_VALUE = 0, MAX_VALUE = 127 };

  unsigned deviceId;
  int channel;
  int key;
  int low_value;
  int high_value;
  bool useNoteOff;

  GOMidiEventPattern(
    unsigned uDeviceId,
    int iChannel,
    int iKey,
    int iLowValue = 0,
    int iHighValue = 0,
    bool iUseNoteOff = true)
    : deviceId(uDeviceId),
      channel(iChannel),
      key(iKey),
      low_value(iLowValue),
      high_value(iHighValue),
      useNoteOff(iUseNoteOff) {}

  bool operator==(const GOMidiEventPattern &other) const;

  /**
   * Convert a midi value between ranges: from [srcLow, srcHigh] to
   *   [dstLow, dstHigh]. All ranges must be between 0 and 127. They may be
   *   normal (low < high) or inversed (high < low)
   * @param srcValue - source midi value
   * @param srcLow - the low bound of the source range
   * @param srcHigh - the high bound of the source range
   * @param dstLow - the low bound of the destination range
   * @param dstHigh - the high bound of the destination range
   * @return the destination value
   **/
  static int convertValueBetweenRanges(
    int srcValue, int srcLow, int srcHigh, int dstLow, int dstHigh);
};

#endif /* GOMIDIEVENTPATTERN_H */
