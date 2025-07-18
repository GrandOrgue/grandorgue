/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiDialogCreatorProxy.h"

void GOMidiDialogCreatorProxy::ShowMIDIEventDialog(
  void *element,
  const wxString &title,
  const wxString &dialogSelector,
  GOMidiObject &obj,
  GOMidiDialogListener *pDialogListener) {
  if (p_creator)
    p_creator->ShowMIDIEventDialog(
      element, title, dialogSelector, obj, pDialogListener);
}
