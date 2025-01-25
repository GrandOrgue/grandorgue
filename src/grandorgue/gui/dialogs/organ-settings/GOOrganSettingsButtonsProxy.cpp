/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganSettingsButtonsProxy.h"

#include <wx/intl.h>

#include "GOEvent.h"

GOOrganSettingsButtonsProxy::GOOrganSettingsButtonsProxy(
  Listener &listener, wxWindow *pWindow)
  : r_listener(listener), p_window(pWindow) {}

void GOOrganSettingsButtonsProxy::NotifyModified(bool newValue) {
  SetModified(newValue);
  NotifyButtonStatesChanged();
}

bool GOOrganSettingsButtonsProxy::CheckForUnapplied() {
  bool res = IsModified();

  if (res)
    GOMessageBox(
      _("Please apply or discard changes first"),
      _("Error"),
      wxOK | wxICON_ERROR,
      p_window);
  return res;
}
