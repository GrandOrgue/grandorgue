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
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    GOMidiSenderType senderType,
    GOMidiReceiverType receiverType);

  ~GOMidiObjectWithDivision();
};

#endif /* GOMIDIOBJECTWITHDIVISION_H */
