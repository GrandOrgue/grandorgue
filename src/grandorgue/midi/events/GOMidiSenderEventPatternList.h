/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDEREVENTPATTERNLIST_H
#define GOMIDISENDEREVENTPATTERNLIST_H

#include "GOMidiEventPatternList.h"
#include "GOMidiSenderEventPattern.h"
#include "GOMidiSenderType.h"

using GOMidiSenderEventPatternList
  = GOMidiEventPatternList<GOMidiSenderType, GOMidiSenderEventPattern>;

#endif
