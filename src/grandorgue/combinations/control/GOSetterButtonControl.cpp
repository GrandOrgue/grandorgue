/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSetterButtonControl.h"

#include <wx/intl.h>

#include "GOSetterButtonCallback.h"

GOSetterButtonControl::GOSetterButtonControl(
  GODefinitionFile *organfile, GOSetterButtonCallback *setter, bool Pushbutton)
  : GOButtonControl(organfile, MIDI_RECV_SETTER, Pushbutton),
    m_setter(setter) {}

void GOSetterButtonControl::Push() {
  if (m_Pushbutton)
    m_setter->SetterButtonChanged(this);
  else
    GOButtonControl::Push();
}

void GOSetterButtonControl::Set(bool on) {
  if (IsEngaged() == on)
    return;
  m_setter->SetterButtonChanged(this);
  Display(on);
}

wxString GOSetterButtonControl::GetMidiType() { return _("Button"); }
