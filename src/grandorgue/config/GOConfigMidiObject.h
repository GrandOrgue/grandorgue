/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONFIGMIDIOBJECT_H
#define GOCONFIGMIDIOBJECT_H

#include "midi/events/GOMidiReceiverType.h"
#include "midi/events/GOMidiSenderType.h"
#include "midi/events/GOMidiShortcutReceiverType.h"
#include "midi/objects/GOMidiObject.h"

/** It is a dummy MIDI object that is used only for storing MIDI configuration
 */

class GOConfigMidiObject : public GOMidiObject {
private:
  wxString m_path;
  GOMidiSender *mp_MidiSender;
  GOMidiReceiver *mp_MidiReceiver;
  GOMidiShortcutReceiver *mp_ShortcutReceiver;
  GOMidiSender *mp_DivisionSender;

  template <typename MidiElementType>
  void ClearMidiElement(MidiElementType *pMidiElement);

  template <typename MidiElementType>
  void ReplaceMidiElement(
    MidiElementType *pNewElement,
    MidiElementType *&slot,
    void (GOMidiObject::*setPointerFun)(MidiElementType *pNewElement));

  void ReplaceMidiSender(GOMidiSender *pNewElement) {
    ReplaceMidiElement(
      pNewElement, mp_MidiSender, &GOConfigMidiObject::SetMidiSender);
  }

  void ReplaceMidiReceiver(GOMidiReceiver *pNewElement) {
    ReplaceMidiElement(
      pNewElement, mp_MidiReceiver, &GOConfigMidiObject::SetMidiReceiver);
  }

  void ReplaceShortcutReceiver(GOMidiShortcutReceiver *pNewElement) {
    ReplaceMidiElement(
      pNewElement,
      mp_ShortcutReceiver,
      &GOConfigMidiObject::SetMidiShortcutReceiver);
  }

  void ReplaceDivisionSender(GOMidiSender *pNewElement) {
    ReplaceMidiElement(
      pNewElement, mp_DivisionSender, &GOConfigMidiObject::SetDivisionSender);
  }

public:
  GOConfigMidiObject(
    GOMidiMap &midiMap,
    ObjectType objectType,
    GOMidiSenderType senderType,
    GOMidiReceiverType receiverType,
    GOMidiShortcutReceiverType shortcutType,
    bool hasReceiver,
    bool hasShortcut,
    bool hasDivision);

  ~GOConfigMidiObject();
};

#endif /* GOCONFIGMIDIOBJECT_H */
