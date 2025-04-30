/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiConfigDispatcher.h"

#include <algorithm>

void GOMidiConfigDispatcher::RemoveListener(GOMidiDialogListener *pListener) {
  mp_listeners.erase(
    std::remove(mp_listeners.begin(), mp_listeners.end(), pListener),
    mp_listeners.end());
}

void GOMidiConfigDispatcher::OnSettingsApplied() {
  for (auto pL : mp_listeners)
    pL->OnSettingsApplied();
}
