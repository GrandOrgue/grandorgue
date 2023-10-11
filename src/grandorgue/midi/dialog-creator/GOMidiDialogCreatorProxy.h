/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIDIALOGCREATORPROXY_H
#define GOMIDIDIALOGCREATORPROXY_H

#include "GOMidiDialogCreator.h"

class GOMidiDialogCreatorProxy : public GOMidiDialogCreator {
private:
  GOMidiDialogCreator *p_creator = nullptr;

public:
  void SetMidiDialogCreator(GOMidiDialogCreator *pCreator) {
    p_creator = pCreator;
  }

  void ShowMIDIEventDialog(
    void *element,
    const wxString &title,
    const wxString &dialogSelector,
    GOMidiReceiverBase *event,
    GOMidiSender *sender,
    GOMidiShortcutReceiver *key,
    GOMidiSender *division = nullptr) override;
};

#endif /* GOMIDIDIALOGCREATORPROXY_H */
