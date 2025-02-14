/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINGSDIALOGBASE_H
#define GOORGANSETTINGSDIALOGBASE_H

#include "gui/dialogs/common/GOTabbedDialog.h"

class GOOrganSettingsDialogBase : public GOTabbedDialog {
public:
  virtual void ButtonStatesChanged() = 0;
};

#endif /* GOORGANSETTINGSDIALOGBASE_H */
