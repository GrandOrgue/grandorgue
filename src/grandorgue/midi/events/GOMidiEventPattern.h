/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTPATTERN_H
#define GOMIDIEVENTPATTERN_H

#include <cstdint>

#include "GOStringSet.h"

namespace YAML {
class Node;
};

class GOMidiMap;

struct GOMidiEventPattern {
  enum { MIN_VALUE = 0, MAX_VALUE = 127 };

  uint16_t deviceId;
  int8_t channel;
  int8_t key;
  int8_t low_value;
  int8_t high_value;
  bool useNoteOff;

  GOMidiEventPattern(
    uint_fast16_t uDeviceId,
    int_fast8_t iChannel,
    int_fast8_t iKey,
    int_fast8_t iLowValue = 0,
    int_fast8_t iHighValue = 0,
    bool iUseNoteOff = true)
    : deviceId(uDeviceId),
      channel(iChannel),
      key(iKey),
      low_value(iLowValue),
      high_value(iHighValue),
      useNoteOff(iUseNoteOff) {}

  virtual bool IsEmpty() const = 0;

  bool operator==(const GOMidiEventPattern &other) const;

  void DeviceIdToYaml(YAML::Node &eventNode, const GOMidiMap &map) const;
  void DeviceIdFromYaml(
    const YAML::Node &eventNode,
    const wxString &eventPath,
    GOMidiMap &map,
    GOStringSet &unusedPaths);

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
    int_fast8_t srcValue,
    int_fast8_t srcLow,
    int_fast8_t srcHigh,
    int_fast8_t dstLow,
    int_fast8_t dstHigh);
};

#endif /* GOMIDIEVENTPATTERN_H */
