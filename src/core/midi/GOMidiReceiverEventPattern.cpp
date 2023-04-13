/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiReceiverEventPattern.h"

bool GOMidiReceiverEventPattern::operator==(
  const GOMidiReceiverEventPattern &other) const {
  return (const GOMidiEventPattern &)*this == (const GOMidiEventPattern &)other
    && type == other.type && low_key == other.low_key
    && high_key == other.high_key && debounce_time == other.debounce_time;
}
