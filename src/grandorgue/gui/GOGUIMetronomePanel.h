/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIMETRONOMEPANEL_H
#define GOGUIMETRONOMEPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUIMetronomePanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateMetronomePanel(GOConfigReader &cfg);

public:
  GOGUIMetronomePanel(GOOrganController *organController);
  virtual ~GOGUIMetronomePanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
