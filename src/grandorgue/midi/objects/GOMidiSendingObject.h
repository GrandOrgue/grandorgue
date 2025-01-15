/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDINGOBJECT_H
#define GOMIDISENDINGOBJECT_H

#include "midi/GOMidiSender.h"

#include "GOMidiObject.h"

class GOOrganModel;

class GOMidiSendingObject : public GOMidiObject {
private:
  GOMidiSender m_sender;

protected:
  GOMidiSendingObject(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    GOMidiSenderType senderType,
    GOMidiReceiverBase *pMidiReceiver = nullptr,
    GOMidiShortcutReceiver *pShortcutReceiver = nullptr,
    GOMidiSender *pDivisionSender = nullptr);

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  void SaveMidiObject(
    GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) override;

  virtual void SetElementId(int id);

  void SendMidiValue(bool value) { m_sender.SetDisplay(value); }
  void SendMidiValue(int value) { m_sender.SetValue(value); }
  void SendMidiValue(const wxString &value) { m_sender.SetLabel(value); }
  void SendMidiKey(unsigned key, unsigned value) {
    m_sender.SetKey(key, value);
  }
  void ResetMidiKey() { m_sender.ResetKey(); }

  void PreparePlayback() override;
  void AbortPlayback() override;
};

#endif /* GOMIDISENDINGOBJECT_H */
