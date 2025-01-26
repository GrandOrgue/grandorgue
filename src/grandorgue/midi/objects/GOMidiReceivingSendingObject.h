/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVINGSENDINGOBJECT_H
#define GOMIDIRECEIVINGSENDINGOBJECT_H

#include "midi/GOMidiReceiver.h"

#include "GOEventHandler.h"
#include "GOMidiSendingObject.h"

class GOMidiReceivingSendingObject : public GOMidiSendingObject,
                                     private GOEventHandler {
private:
  GOMidiReceiver m_receiver;
  const GOMidiReceiver::KeyMap *p_ReceiverKeyMap;

protected:
  GOMidiReceivingSendingObject(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    GOMidiSenderType senderType,
    GOMidiReceiverType reveiverType);

  ~GOMidiReceivingSendingObject();

  const GOMidiReceiver::KeyMap *GetReceiverKeyMap() { return p_ReceiverKeyMap; }
  void SetReceiverKeyMap(const GOMidiReceiver::KeyMap *pKeyMap) {
    p_ReceiverKeyMap = pKeyMap;
  }

public:
  // Should be used before Load()
  void SetInitialMidiIndex(unsigned index) { m_receiver.SetIndex(index); }

  virtual void SetElementId(int id) override;

protected:
  virtual void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  virtual void SaveMidiObject(
    GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) override;

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
