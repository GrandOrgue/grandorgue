/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiDialogCreatorProxy.h"

void GOMidiDialogCreatorProxy::ShowMIDIEventDialog(
  void *element,
  const wxString &title,
  const wxString &dialogSelector,
  GOMidiReceiverBase *event,
  GOMidiSender *sender,
  GOMidiShortcutReceiver *key,
  GOMidiSender *division) {
  if (p_creator)
    p_creator->ShowMIDIEventDialog(
      element, title, dialogSelector, event, sender, key, division);
}
