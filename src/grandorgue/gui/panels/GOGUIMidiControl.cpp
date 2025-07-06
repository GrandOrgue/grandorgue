/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "midi/objects/GOMidiPlayingObject.h"

#include "GOGUIMidiControl.h"

bool GOGUIMidiControl::HandleMidiConfig(bool isRightClick) {

  if (isRightClick && p_MidiObject)
    p_MidiObject->ShowConfigDialog();
  return isRightClick;
}