/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPUSHBUTTONCONTROL_H
#define GOPUSHBUTTONCONTROL_H

#include "GOButtonControl.h"

class GOPushbuttonControl : public GOButtonControl {
public:
  GOPushbuttonControl(GOOrganModel &organModel)
    : GOButtonControl(organModel, MIDI_RECV_BUTTON, true) {}
};

#endif
