/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLabelControl.h"

#include <wx/intl.h>

#include "model/GOOrganModel.h"

const wxString WX_MIDI_TYPE_CODE = wxT("Label");
const wxString WX_MIDI_TYPE_NAME = _("Label");

GOLabelControl::GOLabelControl(GOOrganModel &organModel)
  : GOMidiSendingObject(
    organModel, WX_MIDI_TYPE_CODE, WX_MIDI_TYPE_NAME, MIDI_SEND_LABEL) {}

void GOLabelControl::SetContent(const wxString &name) {
  m_Content = name;
  SendCurrentMidiValue();
  r_OrganModel.SendControlChanged(this);
}
