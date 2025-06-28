/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVINGSENDINGOBJECT_H
#define GOMIDIRECEIVINGSENDINGOBJECT_H

#include "midi/elements/GOMidiReceiver.h"

#include "GOEventHandler.h"
#include "GOMidiSendingObject.h"

class GOMidiReceivingSendingObject : public GOMidiSendingObject,
                                     private GOEventHandler {
private:
  GOMidiReceiverType m_ReceiverType;
  GOMidiReceiver m_receiver;
  const GOMidiReceiver::KeyMap *p_ReceiverKeyMap;
  // used for load initial config
  int m_MidiInputNumber;

protected:
  GOMidiReceivingSendingObject(
    GOOrganModel &organModel,
    ObjectType objectType,
    GOMidiSenderType senderType,
    GOMidiReceiverType receiverType);

  ~GOMidiReceivingSendingObject();

  const GOMidiReceiver::KeyMap *GetReceiverKeyMap() { return p_ReceiverKeyMap; }
  void SetReceiverKeyMap(const GOMidiReceiver::KeyMap *pKeyMap) {
    p_ReceiverKeyMap = pKeyMap;
  }

public:
  // Should be used before Load()
  int GetMidiInputNumber() const { return m_MidiInputNumber; }
  void SetMidiInputNumber(int midiInputNumber) {
    m_MidiInputNumber = midiInputNumber;
  }

  virtual void SetElementId(int id) override;

protected:
  void AfterMidiLoaded() override;

  // Now it is present only for the symmetry with Load
  // TODO: add the initialMidiNumber parameter
  void Init(
    GOConfigReader &cfg, const wxString &group, const wxString &name) override;
  using GOMidiSendingObject::Load;
  void Load(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &name,
    bool mayHaveOdfMidiInputNumber);

  void PreparePlayback() override;

private:
  void ProcessMidi(const GOMidiEvent &event) override;

protected:
  virtual void OnMidiReceived(
    const GOMidiEvent &event, GOMidiMatchType matchType, int key, int value)
    = 0;

  virtual void HandleKey(int key) override {}
};

#endif /* GOMIDIRECEIVINGSENDINGOBJECT_H */
