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
  const wxString &r_ObjectGroup;
  wxString m_MatchingBy;
  GOMidiSender *mp_MidiSender;
  GOMidiReceiver *mp_MidiReceiver;
  GOMidiShortcutReceiver *mp_ShortcutReceiver;
  GOMidiSender *mp_DivisionSender;

  template <typename MidiElementType>
  void ClearMidiElement(MidiElementType *pMidiElement);

  template <typename MidiElementType, typename MidiElementTypeEnum>
  void SetElementType(
    int newElementType,
    MidiElementType *&slot,
    void (GOMidiObject::*setPointerFun)(MidiElementType *pNewElement));

public:
  GOConfigMidiObject(
    GOMidiMap &midiMap,
    ObjectType objectType,
    const wxString &objectGgroup,
    const wxString &matchingBy,
    const wxString &name);

  ~GOConfigMidiObject();

  const wxString &GetObjectGroup() const { return r_ObjectGroup; }
  const wxString &GetMatchingBy() const { return m_MatchingBy; }

  void SetSenderType(int senderType);
  void SetReceiverType(int receiverType);
  void SetShortcutReceiverType(int shortcutReceiverType);
  // Only MIDI_SEND_MANUAL and ELEMENT_TYPE_NONE are allowed
  void SetDivisionSenderType(int senderType);

  void AssignFrom(const GOMidiObject &objFrom);

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;

  void SaveMidiObject(
    GOConfigWriter &cfg,
    const wxString &group,
    GOMidiMap &midiMap) const override;
};

#endif /* GOCONFIGMIDIOBJECT_H */
