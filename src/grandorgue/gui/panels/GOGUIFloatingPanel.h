/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIFLOATINGPANEL_H
#define GOGUIFLOATINGPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUIFloatingPanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateFloatingPanel(GOConfigReader &cfg);

public:
  GOGUIFloatingPanel(GOOrganController *organController);
  ~GOGUIFloatingPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
