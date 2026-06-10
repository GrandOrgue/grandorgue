/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventPattern.h"

#include <algorithm>
#include <math.h>

#include <wx/intl.h>
#include <wx/log.h>

#include "yaml/go-wx-yaml.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "midi/GOMidiMap.h"

bool GOMidiEventPattern::operator==(const GOMidiEventPattern &other) const {
  return deviceId == other.deviceId && channel == other.channel
    && key == other.key && low_value == other.low_value
    && high_value == other.high_value && useNoteOff == other.useNoteOff;
}

static wxString WX_DEVICE = wxT("device");

void GOMidiEventPattern::FillDeviceId(
  const wxString &msgContext, const wxString &deviceName, GOMidiMap &map) {
  deviceId = map.EnsureLogicalName(deviceName, [=](GONameMap::IdType id) {
    wxLogWarning(
      _("%s: Unknown MIDI device logical name \"%s\""), msgContext, deviceName);
  });
}

static wxString calc_key(const wxString &keyPrefix, unsigned patternIndex) {
  return wxString::Format(wxT("%s%03u"), keyPrefix, patternIndex + 1);
}

void GOMidiEventPattern::LoadDeviceId(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &keyPrefix,
  unsigned patternIndex,
  GOMidiMap &map) {
  wxString key = calc_key(keyPrefix, patternIndex);
  wxString deviceName = cfg.ReadString(CMBSetting, group, key, false);

  FillDeviceId(wxString::Format(wxT("%s/%s"), group, key), deviceName, map);
}

void GOMidiEventPattern::SaveDeviceId(
  GOConfigWriter &cfg,
  const wxString &group,
  const wxString &keyPrefix,
  unsigned patternIndex,
  const GOMidiMap &map) const {
  cfg.WriteString(
    group,
    calc_key(keyPrefix, patternIndex),
    map.GetDeviceLogicalNameById(deviceId));
}

void GOMidiEventPattern::DeviceIdToYaml(
  YAML::Node &eventNode, const GOMidiMap &map) const {
  if (deviceId)
    eventNode[WX_DEVICE] = map.GetDeviceLogicalNameById(deviceId);
}

void GOMidiEventPattern::DeviceIdFromYaml(
  const YAML::Node &eventNode,
  const wxString &eventPath,
  GOMidiMap &map,
  GOStringSet &unusedPaths) {
  FillDeviceId(
    eventPath,
    read_string(eventNode, eventPath, WX_DEVICE, false, unusedPaths),
    map);
}

int GOMidiEventPattern::convertValueBetweenRanges(
  int_fast8_t srcValue,
  int_fast8_t srcLow,
  int_fast8_t srcHigh,
  int_fast8_t dstLow,
  int_fast8_t dstHigh) {
  // Linear interpolation from [srcLow, srcHigh] to [dstLow, dstHigh].
  // dstValue is int (not int_fast8_t) to avoid overflow when the intermediate
  // result exceeds 127 (e.g. srcValue > srcHigh maps to dstHigh + epsilon).
  const int dstValue = srcHigh != srcLow ? dstLow
      + (int)round((srcValue - srcLow) * (float)(dstHigh - dstLow)
                   / (srcHigh - srcLow))
                                         : (int)dstLow;

  // Clamp to [min(dstLow,dstHigh), max(dstLow,dstHigh)] to handle both normal
  // (low < high) and inverted (high < low) destination ranges, and to keep
  // out-of-range srcValues (srcValue < srcLow or srcValue > srcHigh) within
  // the destination bounds.
  return std::clamp(
    dstValue, (int)std::min(dstLow, dstHigh), (int)std::max(dstLow, dstHigh));
}
