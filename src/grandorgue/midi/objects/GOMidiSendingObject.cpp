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
  GOMidiSenderType senderType)
  : GOMidiObject(organModel, midiTypeCode, midiTypeName),
    m_sender(organModel, senderType) {
  SetMidiSender(&m_sender);
}

GOMidiSendingObject::~GOMidiSendingObject() { SetMidiSender(nullptr); }

void GOMidiSendingObject::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiObject::LoadMidiObject(cfg, group, midiMap);
  m_sender.Load(cfg, group, midiMap);
}

void GOMidiSendingObject::SetElementId(int id) { m_sender.SetElementID(id); }

void GOMidiSendingObject::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) const {
  GOMidiObject::SaveMidiObject(cfg, group, midiMap);
  m_sender.Save(cfg, group, midiMap);
}

void GOMidiSendingObject::ResendMidi() {
  m_sender.SetName(GetName());
  SendCurrentMidiValue();
}

void GOMidiSendingObject::PreparePlayback() {
  GOMidiObject::PreparePlayback();
  m_sender.SetName(GetName());
}

void GOMidiSendingObject::PrepareRecording() {
  GOMidiObject::PrepareRecording();
  SendCurrentMidiValue();
}

void GOMidiSendingObject::AbortPlayback() {
  SendEmptyMidiValue();
  GOMidiObject::AbortPlayback();
  m_sender.SetName(wxEmptyString);
}
