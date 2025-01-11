/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObject.h"

#include <wx/intl.h>

#include "midi/dialog-creator/GOMidiDialogCreator.h"

void GOMidiObject::ShowConfigDialog() {
  const wxString &midiTypeCode = GetMidiTypeCode();
  const wxString &midiName = GetMidiName();
  wxString title
    = wxString::Format(_("Midi-Settings for %s - %s"), GetMidiType(), midiName);
  wxString selector = wxString::Format(wxT("%s.%s"), midiTypeCode, midiName);

  r_DialogCreator.ShowMIDIEventDialog(
    this,
    title,
    selector,
    GetMidiReceiver(),
    GetMidiSender(),
    GetMidiShortcutReceiver(),
    GetDivision());
}
