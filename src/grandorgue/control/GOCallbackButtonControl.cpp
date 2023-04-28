/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCallbackButtonControl.h"

#include <wx/intl.h>

#include "control/GOButtonCallback.h"

GOCallbackButtonControl::GOCallbackButtonControl(
  GOOrganController *organController,
  GOButtonCallback *callback,
  bool isPushbutton,
  bool isPiston)
  : GOButtonControl(organController, MIDI_RECV_SETTER, isPushbutton, isPiston),
    m_callback(callback) {}

void GOCallbackButtonControl::Push() {
  if (m_Pushbutton)
    m_callback->ButtonStateChanged(this, true);
  else
    GOButtonControl::Push();
}

void GOCallbackButtonControl::Set(bool on) {
  if (IsEngaged() == on)
    return;
  m_callback->ButtonStateChanged(this, on);
  Display(on);
}

wxString GOCallbackButtonControl::GetMidiType() { return _("Button"); }
