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

typedef enum {
  MIDI_RECV_DRAWSTOP,
  MIDI_RECV_BUTTON,
  MIDI_RECV_ENCLOSURE,
  MIDI_RECV_MANUAL,
  MIDI_RECV_SETTER,
  MIDI_RECV_ORGAN,
} GOMidiReceiverType;

typedef enum {
  MIDI_MATCH_NONE,
  MIDI_MATCH_ON,
  MIDI_MATCH_OFF,
  MIDI_MATCH_CHANGE,
  MIDI_MATCH_RESET,
} GOMidiMatchType;

using GOMidiReceiverEventPatternList
  = GOMidiEventPatternList<GOMidiReceiverType, GOMidiReceiverEventPattern>;

#endif
