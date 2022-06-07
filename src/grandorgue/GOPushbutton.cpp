/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPushbutton.h"

GOPushbutton::GOPushbutton(GODefinitionFile *organfile)
  : GOButton(organfile, MIDI_RECV_BUTTON, true) {}
