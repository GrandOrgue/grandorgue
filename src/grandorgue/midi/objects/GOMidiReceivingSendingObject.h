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

protected:
  GOMidiReceivingSendingObject(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    GOMidiSenderType senderType,
    GOMidiReceiverType reveiverType,
    GOMidiShortcutReceiver *pShortcutReceiver = nullptr,
    GOMidiSender *pDivisionSender = nullptr);

  ~GOMidiReceivingSendingObject();

  virtual void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  virtual void SaveMidiObject(
    GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) override;

  void PreparePlayback() override;

  void ProcessMidi(const GOMidiEvent &event) override;

  virtual void OnMidiReceived(
    const GOMidiEvent &event, GOMidiMatchType matchType, int value)
    = 0;

public:
  virtual void SetElementId(int id) override;
  void SetReceiverIndex(unsigned index) { m_receiver.SetIndex(index); }
};

#endif /* GOMIDIRECEIVINGSENDINGOBJECT_H */
