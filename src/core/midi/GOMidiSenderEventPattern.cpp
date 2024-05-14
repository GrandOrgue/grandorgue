/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSenderEventPattern.h"

bool GOMidiSenderEventPattern::operator==(
  const GOMidiSenderEventPattern &other) const {
  return (const GOMidiEventPattern &)*this == (const GOMidiEventPattern &)other
    && type == other.type && start == other.start && length == other.length;
}
