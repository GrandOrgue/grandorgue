/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganSettingsButtonsProxy.h"

void GOOrganSettingsButtonsProxy::NotifyModified(bool newValue) {
  SetModified(newValue);
  NotifyButtonStatesChanged();
}