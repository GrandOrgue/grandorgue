/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMetronome.h"

GOSettingsMetronome::GOSettingsMetronome(
  GOConfig &config,
  GOTabbedDialog *pDlg,
  const wxString &name,
  const wxString &label)
  : GODialogTab(pDlg, name, label), r_config(config) {}