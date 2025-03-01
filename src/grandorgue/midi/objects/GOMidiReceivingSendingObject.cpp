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
  GOMidiReceiverType reveiverType)
  : GOMidiSendingObject(organModel, midiTypeCode, midiTypeName, senderType),
    m_receiver(organModel, reveiverType),
    p_ReceiverKeyMap(nullptr),
    m_MidiInputNumber(0) {
  SetMidiReceiver(&m_receiver);
  r_OrganModel.RegisterEventHandler(this);
}

GOMidiReceivingSendingObject::~GOMidiReceivingSendingObject() {
  r_OrganModel.UnRegisterEventHandler(this);
  SetMidiReceiver(nullptr);
}

void GOMidiReceivingSendingObject::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  // using in Load for initial MIDI config
  m_MidiInputNumber = 0;
  GOMidiSendingObject::Init(cfg, group, name);
}

void GOMidiReceivingSendingObject::Load(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  // using in Load for initial MIDI config
  m_MidiInputNumber = cfg.ReadInteger(
    ODFSetting, group, wxT("MIDIInputNumber"), 0, 200, false, 0);
  GOMidiSendingObject::Load(cfg, group, name);
}

void GOMidiReceivingSendingObject::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiSendingObject::LoadMidiObject(cfg, group, midiMap);
  if (!IsReadOnly()) {
    m_receiver.Load(cfg, group, midiMap, m_MidiInputNumber);
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
    int key;
    int value;
    GOMidiMatchType matchType
      = m_receiver.Match(event, p_ReceiverKeyMap, key, value);

    if (matchType > MIDI_MATCH_NONE)
      OnMidiReceived(event, matchType, key, value);
  }
}
