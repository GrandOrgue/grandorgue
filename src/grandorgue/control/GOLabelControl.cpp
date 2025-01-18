/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLabelControl.h"

#include <wx/intl.h>

#include "model/GOOrganModel.h"

GOLabelControl::GOLabelControl(GOOrganModel &organModel)
  : GOMidiSendingObject(
    *organModel, WX_MIDI_TYPE_CODE, WX_MIDI_TYPE_NAME, MIDI_SEND_LABEL) {}

const wxString &GOLabelControl::GetContent() { return m_Content; }

void GOLabelControl::SetContent(wxString name) {
  m_Content = name;
  SendMidiValue(m_Content);
  r_OrganModel.SendControlChanged(this);
}

void GOLabelControl::AbortPlayback() {
  SendMidiValue(wxEmptyString);
  GOMidiSendingObject::AbortPlayback();
}

void GOLabelControl::PrepareRecording() {
  GOMidiSendingObject::PrepareRecording();
  SendMidiValue(m_Content);
}

wxString GOLabelControl::GetElementStatus() { return m_Content; }

std::vector<wxString> GOLabelControl::GetElementActions() {
  std::vector<wxString> actions;
  return actions;
}

void GOLabelControl::TriggerElementActions(unsigned no) {
  // Never called
}
