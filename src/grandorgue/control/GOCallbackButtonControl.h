/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCALLBACKBUTTONCONTROL_H
#define GOCALLBACKBUTTONCONTROL_H

#include "GOButtonControl.h"

class GOButtonCallback;

class GOCallbackButtonControl : public GOButtonControl {
public:
  static const wxString WX_MIDI_TYPE_CODE;
  static const wxString WX_MIDI_TYPE_NAME;

protected:
  GOButtonCallback *m_callback;

public:
  GOCallbackButtonControl(
    GOOrganModel &organModel,
    GOButtonCallback *callback,
    bool isPushbutton,
    bool isPiston = false);
  void Push(void) override;
  void SetButtonState(bool on) override;
};

#endif
