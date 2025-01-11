/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCallbackButtonControl.h"

#include <wx/intl.h>

#include "control/GOButtonCallback.h"

static const wxString WX_MIDI_TYPE_CODE = wxT("Button");
static const wxString WX_MIDI_TYPE_NAME = _("Button");

GOCallbackButtonControl::GOCallbackButtonControl(
  GOOrganModel &organModel,
  GOButtonCallback *callback,
  bool isPushbutton,
  bool isPiston)
  : GOButtonControl(
    organModel,
    WX_MIDI_TYPE_CODE,
    WX_MIDI_TYPE_NAME,
    MIDI_RECV_SETTER,
    isPushbutton,
    isPiston),
    m_callback(callback) {}

void GOCallbackButtonControl::Push() {
  if (m_Pushbutton)
    m_callback->ButtonStateChanged(this, true);
  else
    GOButtonControl::Push();
}

void GOCallbackButtonControl::SetButtonState(bool on) {
  if (IsEngaged() == on)
    return;
  m_callback->ButtonStateChanged(this, on);
  Display(on);
}
