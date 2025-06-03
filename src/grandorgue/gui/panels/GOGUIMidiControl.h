/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIMIDICONTROL_H
#define GOGUIMIDICONTROL_H

#include "GOGUIControl.h"

class GOMidiPlayingObject;

/**
 * A Gui control related with a MIDI object
 */
class GOGUIMidiControl : public GOGUIControl {
private:
  GOMidiPlayingObject *p_MidiObject;

public:
  GOGUIMidiControl(GOGUIPanel *pPanel, GOMidiPlayingObject *pMidiObject)
    : GOGUIControl(pPanel, pMidiObject), p_MidiObject(pMidiObject) {}

  /**
   * Opens a MIDI configuration dialog on a right mouse click
   * @param isRightClick is it a right mouse click
   * @return the value of isRightClick
   */
  bool HandleMidiConfig(bool isRightClick);
};

#endif /* GOGUIMIDICONTROL_H */
