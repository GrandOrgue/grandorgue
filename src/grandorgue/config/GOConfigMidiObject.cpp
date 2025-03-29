/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOConfigMidiObject.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "midi/elements/GOMidiReceiver.h"
#include "midi/elements/GOMidiSender.h"
#include "midi/elements/GOMidiShortcutReceiver.h"

static const wxString WX_SENDER_TYPE = wxT("SenderType");
static const wxString WX_RECEIVER_TYPE = wxT("ReceiverType");
static const wxString WX_SHORTCUT_RECEIVER_TYPE = wxT("ShortcutReceiverType");
static const wxString WX_DIVISIONAL_SENDER_TYPE = wxT("DivisionalSenderType");

template <typename MidiElementType>
void GOConfigMidiObject::ClearMidiElement(MidiElementType *pMidiElement) {
  if (pMidiElement)
    delete pMidiElement;
}

template <typename MidiElementType, typename MidiElementTypeEnum>
void GOConfigMidiObject::SetElementType(
  int newElementType,
  MidiElementType *&slot,
  void (GOMidiObject::*setPointerFun)(MidiElementType *pNewElement)) {
  if (getElementType(slot) != newElementType) {
    MidiElementType *pEl = newElementType == ELEMENT_TYPE_NONE
      ? nullptr
      : new MidiElementType((MidiElementTypeEnum)newElementType);

    ClearMidiElement(slot);
    slot = pEl;
    (this->*setPointerFun)(pEl);
  }
}

GOConfigMidiObject::GOConfigMidiObject(
  GOMidiMap &midiMap,
  ObjectType objectType,
  const wxString &objectGroup,
  const wxString &matchingBy,
  const wxString &name)
  : GOMidiObject(midiMap, objectType),
    r_ObjectGroup(objectGroup),
    m_MatchingBy(matchingBy),
    mp_MidiSender(nullptr),
    mp_MidiReceiver(nullptr),
    mp_ShortcutReceiver(nullptr),
    mp_DivisionSender(nullptr) {
  SetName(name);
}

GOConfigMidiObject::~GOConfigMidiObject() {
  ClearMidiElement(mp_DivisionSender);
  ClearMidiElement(mp_ShortcutReceiver);
  ClearMidiElement(mp_MidiReceiver);
  ClearMidiElement(mp_MidiSender);
}

void GOConfigMidiObject::SetSenderType(int senderType) {
  SetElementType<GOMidiSender, GOMidiSenderType>(
    senderType, mp_MidiSender, &GOConfigMidiObject::SetMidiSender);
}

void GOConfigMidiObject::SetReceiverType(int receiverType) {
  SetElementType<GOMidiReceiver, GOMidiReceiverType>(
    receiverType, mp_MidiReceiver, &GOConfigMidiObject::SetMidiReceiver);
}

void GOConfigMidiObject::SetShortcutReceiverType(int shortcutReceiverType) {
  SetElementType<GOMidiShortcutReceiver, GOMidiShortcutReceiverType>(
    shortcutReceiverType,
    mp_ShortcutReceiver,
    &GOConfigMidiObject::SetMidiShortcutReceiver);
}

// Only MIDI_SEND_MANUAL and ELEMENT_TYPE_NONE are allowed
void GOConfigMidiObject::SetDivisionSenderType(int senderType) {
  SetElementType<GOMidiSender, GOMidiSenderType>(
    senderType, mp_DivisionSender, &GOConfigMidiObject::SetDivisionSender);
}

void GOConfigMidiObject::AssignFrom(const GOMidiObject &objFrom) {
  SetSenderType(objFrom.GetSenderType());
  SetReceiverType(objFrom.GetReceiverType());
  SetShortcutReceiverType(objFrom.GetShortcutReceiverType());
  SetDivisionSenderType(objFrom.GetDivisionSenderType());
  CopyMidiSettingFrom(objFrom);
}

void GOConfigMidiObject::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  const int senderType = cfg.ReadEnum(
    CMBSetting, group, WX_SENDER_TYPE, GOMidiSender::SENDER_TYPES);
  const int receiverType = cfg.ReadEnum(
    CMBSetting,
    group,
    WX_RECEIVER_TYPE,
    GOMidiReceiver::RECEIVER_TYPES,
    false,
    ELEMENT_TYPE_NONE);
  const int shortcutReceiverType = cfg.ReadEnum(
    CMBSetting,
    group,
    WX_SHORTCUT_RECEIVER_TYPE,
    GOMidiShortcutReceiver::SHORTCUT_RECEIVER_TYPES,
    false,
    ELEMENT_TYPE_NONE);
  const int divisionalSenerType = cfg.ReadEnum(
    CMBSetting,
    group,
    WX_DIVISIONAL_SENDER_TYPE,
    GOMidiSender::DIVISIONAL_SENDER_TYPES,
    false,
    ELEMENT_TYPE_NONE);

  SetSenderType(senderType);
  SetReceiverType(receiverType);
  SetShortcutReceiverType(shortcutReceiverType);
  SetDivisionSenderType(divisionalSenerType);

  GOMidiObject::LoadMidiObject(cfg, group, midiMap);
}

void GOConfigMidiObject::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) const {
  if (mp_MidiSender)
    cfg.WriteEnum(
      group, WX_SENDER_TYPE, GOMidiSender::SENDER_TYPES, GetSenderType());
  if (mp_MidiReceiver)
    cfg.WriteEnum(
      group,
      WX_RECEIVER_TYPE,
      GOMidiReceiver::RECEIVER_TYPES,
      GetReceiverType());
  if (mp_ShortcutReceiver)
    cfg.WriteEnum(
      group,
      WX_SHORTCUT_RECEIVER_TYPE,
      GOMidiShortcutReceiver::SHORTCUT_RECEIVER_TYPES,
      GetShortcutReceiverType());
  if (mp_DivisionSender)
    cfg.WriteEnum(
      group,
      WX_DIVISIONAL_SENDER_TYPE,
      GOMidiSender::DIVISIONAL_SENDER_TYPES,
      GetDivisionSenderType());
  GOMidiObject::SaveMidiObject(cfg, group, midiMap);
}
