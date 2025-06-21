/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOBJECTWITHDIVISION_H
#define GOMIDIOBJECTWITHDIVISION_H

#include "GOMidiReceivingSendingObject.h"

class GOMidiObjectWithDivision : public GOMidiReceivingSendingObject {
private:
  GOMidiSender m_DivisionSender;

protected:
  GOMidiObjectWithDivision(
    GOOrganModel &organModel,
    ObjectType objectType,
    GOMidiSenderType senderType,
    GOMidiReceiverType receiverType);

  ~GOMidiObjectWithDivision();

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  void SaveMidiObject(
    GOConfigWriter &cfg,
    const wxString &group,
    GOMidiMap &midiMap) const override;

  void SendDivisionMidiKey(unsigned key, unsigned value) {
    m_DivisionSender.SetKey(key, value);
  }
  void ResetDivisionMidiKey() { m_DivisionSender.ResetKey(); }
};

#endif /* GOMIDIOBJECTWITHDIVISION_H */
