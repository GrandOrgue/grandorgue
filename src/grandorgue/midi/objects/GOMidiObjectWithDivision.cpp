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