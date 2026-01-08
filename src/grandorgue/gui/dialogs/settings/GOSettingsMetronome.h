/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSMETRONOME_H
#define GOSETTINGSMETRONOME_H

#include "gui/dialogs/common/GODialogTab.h"

class wxSpinCtrl;
class wxString;

class GOConfig;

class GOSettingsMetronome : public GODialogTab {
private:
  GOConfig &r_config;

  wxSpinCtrl *m_MetronomeMeasure;
  wxSpinCtrl *m_MetronomeBPM;

public:
  GOSettingsMetronome(
    GOConfig &config,
    GOTabbedDialog *pDlg,
    const wxString &name,
    const wxString &label);

  virtual bool TransferDataToWindow() override;
  virtual bool TransferDataFromWindow() override;
};

#endif /* GOSETTINGSMETRONOME_H */
