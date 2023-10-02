/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVEREVENTPATTERNLIST_H
#define GOMIDIRECEIVEREVENTPATTERNLIST_H

#include "GOMidiEventPatternList.h"
#include "GOMidiReceiverEventPattern.h"
#include "GOMidiReceiverType.h"

using GOMidiReceiverEventPatternList
  = GOMidiEventPatternList<GOMidiReceiverType, GOMidiReceiverEventPattern>;

#endif
