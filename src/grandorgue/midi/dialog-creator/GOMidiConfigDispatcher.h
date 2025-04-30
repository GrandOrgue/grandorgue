/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDICONFIGDISPATCHER_H
#define GOMIDICONFIGDISPATCHER_H

#include <vector>

#include "GOMidiDialogListener.h"

class GOMidiConfigDispatcher : public GOMidiDialogListener {
private:
  std::vector<GOMidiDialogListener *> mp_listeners;

public:
  void AddListener(GOMidiDialogListener *pListener) {
    mp_listeners.push_back(pListener);
  }
  void RemoveListener(GOMidiDialogListener *pListener);

  virtual void OnSettingsApplied() override;
};

#endif /* GOMIDICONFIGDISPATCHER_H */
