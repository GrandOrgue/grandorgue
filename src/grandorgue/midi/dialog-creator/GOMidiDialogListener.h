/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIDIALOGLISTENER_H
#define GOMIDIDIALOGLISTENER_H

class GOMidiDialogListener {
public:
  virtual void OnSettingsApplied() = 0;
};

#endif /* GOMIDIDIALOGLISTENER_H */
