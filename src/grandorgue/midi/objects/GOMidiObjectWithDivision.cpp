/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObjectWithDivision.h"

#include "model/GOOrganModel.h"

GOMidiObjectWithDivision::GOMidiObjectWithDivision(
  GOOrganModel &organModel,
  const wxString &midiTypeCode,
  const wxString &midiTypeName,
  GOMidiSenderType senderType,
  GOMidiReceiverType receiverType)
  : GOMidiReceivingSendingObject(
    organModel, midiTypeCode, midiTypeName, senderType, receiverType),
    m_DivisionSender(organModel, MIDI_SEND_MANUAL) {
  SetDivisionSender(&m_DivisionSender);
}

GOMidiObjectWithDivision::~GOMidiObjectWithDivision() {
  SetDivisionSender(nullptr);
}

static const wxString WX_DIVISION = wxT("Division");

void GOMidiObjectWithDivision::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiReceivingSendingObject::LoadMidiObject(cfg, group, midiMap);
  m_DivisionSender.Load(cfg, group + WX_DIVISION, midiMap);
}

void GOMidiObjectWithDivision::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) const {
  GOMidiReceivingSendingObject::SaveMidiObject(cfg, group, midiMap);
  m_DivisionSender.Save(cfg, group + WX_DIVISION, midiMap);
}
