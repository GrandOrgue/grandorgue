/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSwitch.h"

#include <wx/intl.h>

const wxString WX_MIDI_TYPE_CODE = wxT("Drawstop");
const wxString WX_MIDI_TYPE_NAME = _("Drawstop");

GOSwitch::GOSwitch(GOOrganModel &organModel)
  : GODrawstop(organModel, WX_MIDI_TYPE_CODE, WX_MIDI_TYPE_NAME) {}

void GOSwitch::AssociateWithManual(int manualN, unsigned indexInManual) {
  m_AssociatedManualN = m_AssociatedManualN < -1 ? manualN : -1;
  m_IndexInManual = m_AssociatedManualN >= 0 ? indexInManual : 0;
}
