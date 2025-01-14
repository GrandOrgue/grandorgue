/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSendingObject.h"

#include "model/GOOrganModel.h"

GOMidiSendingObject::GOMidiSendingObject(
  GOOrganModel &organModel,
  const wxString &midiTypeCode,
  const wxString &midiTypeName,
  GOMidiSenderType senderType,
  GOMidiReceiverBase *pMidiReceiver,
  GOMidiShortcutReceiver *pShortcutReceiver,
  GOMidiSender *pDivisionSender)
  : GOMidiObject(
    organModel,
    midiTypeCode,
    midiTypeName,
    &m_sender,
    pMidiReceiver,
    pShortcutReceiver,
    pDivisionSender),
    m_sender(organModel, senderType) {}

void GOMidiSendingObject::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiObject::LoadMidiObject(cfg, group, midiMap);
  m_sender.Load(cfg, group, midiMap);
}

void GOMidiSendingObject::SetElementId(int id) { m_sender.SetElementID(id); }

void GOMidiSendingObject::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiObject::SaveMidiObject(cfg, group, midiMap);
  m_sender.Save(cfg, group, midiMap);
}

void GOMidiSendingObject::PreparePlayback() {
  GOMidiObject::PreparePlayback();
  m_sender.SetName(GetName());
}

void GOMidiSendingObject::AbortPlayback() {
  GOMidiObject::AbortPlayback();
  m_sender.SetName(wxEmptyString);
}
