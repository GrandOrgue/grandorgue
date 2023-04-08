/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDEREVENTPATTERNLIST_H
#define GOMIDISENDEREVENTPATTERNLIST_H

#include "GOMidiEventPatternList.h"
#include "GOMidiSenderEventPattern.h"

typedef enum {
  MIDI_SEND_BUTTON,
  MIDI_SEND_LABEL,
  MIDI_SEND_ENCLOSURE,
  MIDI_SEND_MANUAL,
} GOMidiSenderType;

using GOMidiSenderEventPatternList
  = GOMidiEventPatternList<GOMidiSenderType, GOMidiSenderEventPattern>;

#endif
