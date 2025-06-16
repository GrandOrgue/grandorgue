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
  ObjectType objectType,
  GOMidiSenderType senderType,
  GOMidiReceiverType receiverType)
  : GOMidiSendingObject(organModel, objectType, senderType),
    m_ReceiverType(receiverType),
    m_receiver(receiverType),
    p_ReceiverKeyMap(nullptr),
    m_MidiInputNumber(-1) {
  SetMidiReceiver(&m_receiver);
  r_OrganModel.RegisterEventHandler(this);
}

GOMidiReceivingSendingObject::~GOMidiReceivingSendingObject() {
  r_OrganModel.UnRegisterEventHandler(this);
  SetMidiReceiver(nullptr);
}

void GOMidiReceivingSendingObject::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  GOMidiSendingObject::Init(cfg, group, name);
}

void GOMidiReceivingSendingObject::Load(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &name,
  bool mayHaveOdfMidiInputNumber) {
  if (mayHaveOdfMidiInputNumber)
    // using in Load for initial MIDI config
    m_MidiInputNumber = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("MIDIInputNumber"),
      0,
      200,
      false,
      m_MidiInputNumber);
  GOMidiSendingObject::Load(cfg, group, name);
}

void GOMidiReceivingSendingObject::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiSendingObject::LoadMidiObject(cfg, group, midiMap);
  if (!IsReadOnly()) {
    m_receiver.Load(r_OrganModel.GetConfig().ODFCheck(), cfg, group, midiMap);
    if (!m_receiver.IsMidiConfigured() && m_MidiInputNumber >= 0) {
      const GOMidiReceiver *pInitialEvents
        = r_OrganModel.GetConfig().FindMidiEvent(
          m_ReceiverType, m_MidiInputNumber);

      if (pInitialEvents)
        m_receiver.RenewFrom(*pInitialEvents);
    }
  }
}

void GOMidiReceivingSendingObject::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) const {
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

    GOMidiMatchType matchType = m_receiver.Match(
      event,
      p_ReceiverKeyMap,
      r_OrganModel.GetConfig().Transpose(),
      key,
      value);

    if (matchType > MIDI_MATCH_NONE)
      OnMidiReceived(event, matchType, key, value);
  }
}
