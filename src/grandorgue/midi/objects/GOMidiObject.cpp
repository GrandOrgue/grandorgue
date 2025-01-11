/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObject.h"

#include <wx/intl.h>

#include "midi/dialog-creator/GOMidiDialogCreator.h"

GOMidiObject::GOMidiObject(
  GOMidiDialogCreator &dialogCreator,
  const wxString &midiTypeCode,
  const wxString &midiType,
  const wxString &midiName,
  GOMidiSender *pMidiSender,
  GOMidiReceiverBase *pMidiReceiver,
  GOMidiShortcutReceiver *pShortcutReceiver,
  GOMidiSender *pDivisionSender)
  : r_DialogCreator(dialogCreator),
    r_MidiTypeCode(midiTypeCode),
    r_MidiTypeName(midiType),
    r_MidiName(midiName),
    p_MidiSender(pMidiSender),
    p_MidiReceiver(pMidiReceiver),
    p_ShortcutReceiver(pShortcutReceiver),
    p_DivisionSender(pDivisionSender) {}

void GOMidiObject::ShowConfigDialog() {
  const bool isReadOnly = IsReadOnly();
  const wxString title = wxString::Format(
    _("Midi-Settings for %s - %s"), r_MidiTypeName, r_MidiName);
  const wxString selector
    = wxString::Format(wxT("%s.%s"), r_MidiTypeCode, r_MidiName);

  r_DialogCreator.ShowMIDIEventDialog(
    this,
    title,
    selector,
    isReadOnly ? nullptr : p_MidiReceiver,
    p_MidiSender,
    isReadOnly ? nullptr : p_ShortcutReceiver,
    p_DivisionSender);
}
