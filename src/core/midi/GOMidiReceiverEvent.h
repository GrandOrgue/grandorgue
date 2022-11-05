/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVEREVENT_H
#define GOMIDIRECEIVEREVENT_H

#include "GOMidiReceiverMessageType.h"

struct GOMidiReceiverEvent {
  unsigned deviceId;
  GOMidiReceiverMessageType type;
  int channel;
  int key;
  int low_key;
  int high_key;
  int low_value;
  int high_value;
  unsigned debounce_time;
};

#endif /* GOMIDIRECEIVEREVENT_H */
