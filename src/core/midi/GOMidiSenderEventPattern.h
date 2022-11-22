/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDEREVENT_H
#define GOMIDISENDEREVENT_H

#include "GOMidiSenderMessageType.h"

struct GOMidiSenderEvent {
  unsigned deviceId;
  GOMidiSenderMessageType type;
  unsigned channel;
  unsigned key;
  unsigned low_value;
  unsigned high_value;
  unsigned start;
  unsigned length;
};

#endif /* GOMIDISENDEREVENT_H */
