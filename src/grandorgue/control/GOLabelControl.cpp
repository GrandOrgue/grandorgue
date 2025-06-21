/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLabelControl.h"

#include <wx/intl.h>

#include "model/GOOrganModel.h"

GOLabelControl::GOLabelControl(
  GOOrganModel &organModel, const GOMidiObjectContext *pContext)
  : GOMidiSendingObject(organModel, OBJECT_TYPE_LABEL, MIDI_SEND_LABEL) {
  SetContext(pContext);
}

void GOLabelControl::SetContent(const wxString &name) {
  m_Content = name;
  SendCurrentMidiValue();
  r_OrganModel.SendControlChanged(this);
}
