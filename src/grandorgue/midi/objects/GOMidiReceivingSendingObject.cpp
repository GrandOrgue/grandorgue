/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiReceivingSendingObject.h"

#include "model/GOOrganModel.h"

GOMidiReceivingSendingObject::GOMidiReceivingSendingObject(
  GOOrganModel &organModel,
  const wxString &midiTypeCode,
  const wxString &midiTypeName,
  GOMidiSenderType senderType,
  GOMidiReceiverType reveiverType,
  GOMidiShortcutReceiver *pShortcutReceiver,
  GOMidiSender *pDivisionSender)
  : GOMidiSendingObject(
    organModel,
    midiTypeCode,
    midiTypeName,
    senderType,
    &m_receiver,
    pShortcutReceiver,
    pDivisionSender),
    m_receiver(organModel, reveiverType) {
  r_OrganModel.RegisterEventHandler(this);
}

GOMidiReceivingSendingObject::~GOMidiReceivingSendingObject() {
  r_OrganModel.UnRegisterEventHandler(this);
}

void GOMidiReceivingSendingObject::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiSendingObject::LoadMidiObject(cfg, group, midiMap);
  if (!IsReadOnly()) {
    m_receiver.Load(cfg, group, midiMap);
  }
}

void GOMidiReceivingSendingObject::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiSendingObject::SaveMidiObject(cfg, group, midiMap);
  if (!IsReadOnly()) {
    m_receiver.Save(cfg, group, midiMap);
  }
}

void GOMidiReceivingSendingObject::SetElementId(int id) {
  if (!IsReadOnly()) {
    m_receiver.SetElementID(id);
    GOMidiSendingObject::SetElementId(id);
  }
}

void GOMidiReceivingSendingObject::PreparePlayback() {
  GOMidiSendingObject::PreparePlayback();
  m_receiver.PreparePlayback();
}

void GOMidiReceivingSendingObject::ProcessMidi(const GOMidiEvent &event) {
  if (!IsReadOnly()) {
    int value;
    GOMidiMatchType matchType = m_receiver.Match(event, value);

    if (matchType > MIDI_MATCH_NONE)
      OnMidiReceived(event, matchType, value);
  }
}
