/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIMidiControl.h"

#include <wx/intl.h>

#include "gui/GOGuiOrgan.h"
#include "midi/objects/GOMidiPlayingObject.h"

bool GOGUIMidiControl::HandleMidiConfig(bool isRightClick) {

  if (isRightClick && p_MidiObject) {
    GOGuiOrgan *pOrgan = dynamic_cast<GOGuiOrgan *>(GetDocument());

    if (pOrgan)
      pOrgan->ShowMIDIEventDialog(*p_MidiObject, p_MidiObject);
  }
  return isRightClick;
}
