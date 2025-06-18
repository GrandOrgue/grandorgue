/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOConfigMidiObject.h"

#include "midi/elements/GOMidiReceiver.h"
#include "midi/elements/GOMidiSender.h"
#include "midi/elements/GOMidiShortcutReceiver.h"

template <typename MidiElementType>
void GOConfigMidiObject::ClearMidiElement(MidiElementType *pMidiElement) {
  if (pMidiElement)
    delete pMidiElement;
}

template <typename MidiElementType>
void GOConfigMidiObject::ReplaceMidiElement(
  MidiElementType *pNewElement,
  MidiElementType *&slot,
  void (GOMidiObject::*setPointerFun)(MidiElementType *pNewElement)) {
  ClearMidiElement(slot);
  slot = pNewElement;
  (this->*setPointerFun)(pNewElement);
}

GOConfigMidiObject::GOConfigMidiObject(
  GOMidiMap &midiMap,
  ObjectType objectType,
  GOMidiSenderType senderType,
  GOMidiReceiverType receiverType,
  GOMidiShortcutReceiverType shortcutType,
  bool hasReceiver,
  bool hasShortcut,
  bool hasDivision)
  : GOMidiObject(midiMap, objectType),
    mp_MidiSender(nullptr),
    mp_MidiReceiver(nullptr),
    mp_ShortcutReceiver(nullptr),
    mp_DivisionSender(nullptr) {
  ReplaceMidiSender(new GOMidiSender(senderType));
  if (hasReceiver)
    ReplaceMidiReceiver(new GOMidiReceiver(receiverType));
  if (hasShortcut)
    ReplaceShortcutReceiver(new GOMidiShortcutReceiver(shortcutType));
  if (hasDivision)
    ReplaceDivisionSender(new GOMidiSender(MIDI_SEND_MANUAL));
}

GOConfigMidiObject::~GOConfigMidiObject() {
  ClearMidiElement(mp_DivisionSender);
  ClearMidiElement(mp_ShortcutReceiver);
  ClearMidiElement(mp_MidiReceiver);
  ClearMidiElement(mp_MidiSender);
}
