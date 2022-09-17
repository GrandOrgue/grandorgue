/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCallbackButtonControl.h"

#include <wx/intl.h>

#include "control/GOButtonCallback.h"

GOCallbackButtonControl::GOCallbackButtonControl(
  GODefinitionFile *organfile, GOButtonCallback *setter, bool Pushbutton)
  : GOButtonControl(organfile, MIDI_RECV_SETTER, Pushbutton),
    m_callback(setter) {}

void GOCallbackButtonControl::Push() {
  if (m_Pushbutton)
    m_callback->ButtonStateChanged(this);
  else
    GOButtonControl::Push();
}

void GOCallbackButtonControl::Set(bool on) {
  if (IsEngaged() == on)
    return;
  m_callback->ButtonStateChanged(this);
  Display(on);
}

wxString GOCallbackButtonControl::GetMidiType() { return _("Button"); }
