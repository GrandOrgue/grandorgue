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
  ObjectType objectType,
  GOMidiSenderType senderType,
  GOMidiReceiverType receiverType)
  : GOMidiReceivingSendingObject(
    organModel, objectType, senderType, receiverType),
    m_DivisionSender(MIDI_SEND_MANUAL) {
  m_DivisionSender.SetProxy(&organModel);
  SetDivisionSender(&m_DivisionSender);
}

GOMidiObjectWithDivision::~GOMidiObjectWithDivision() {
  SetDivisionSender(nullptr);
  m_DivisionSender.SetProxy(nullptr);
}
